using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Threading.Tasks;
using Akka.Actor;
using Akka.Event;
using Akka.Util.Internal;
using OmnetServices;

namespace SampleApp
{
    public static class NetworkExtension
    {
        public static INetwork GetNetwork(this ActorSystem sys)
        {
            return Network.Create(sys);
        }
    }

    /// <summary>
    /// A generic network interface.
    /// </summary>
    public interface INetwork
    {
        void Send(IActorRef sender, IActorRef receiver, object message);
        void Broadcast(IActorRef sender, object message);
        void Subscribe(IActorRef subscriber, Type type);
    }

    /// <inheritdoc />
    /// <summary>
    /// A typesafe wrapper around the network actor for omnet.
    /// </summary>
    public class Network : INetwork
    {
        private readonly IActorRef _actor;

        private Network(IActorRef actor)
        {
            _actor = actor;
        }

        public void Send(IActorRef sender, IActorRef receiver, object message)
        {
            _actor.Tell(new NetworkActor.Send(sender, receiver, message));
        }

        public void Broadcast(IActorRef sender, object message)
        {
            _actor.Tell(new NetworkActor.Broadcast(sender, message));
        }

        public void Subscribe(IActorRef subscriber, Type type)
        {
            _actor.Tell(new NetworkActor.Subscribe(subscriber, type));
        }

        public void CreateOmnetNode(IActorRef node)
        {
            _actor.Tell(new NetworkActor.CreateOmnetNode(node));
        }
        
        /// <summary>
        /// IMPORTANT: <see cref="Create"/> can be accessed concurrently, we thus need to create a lock to ensure that
        ///            we always return the same network instance for the same actor system.
        /// </summary>
        private static readonly IDictionary<ActorSystem, IActorRef> Cache = new Dictionary<ActorSystem, IActorRef>();

        public static INetwork Create(ActorSystem sys)
        {
            // Handle concurrent accesses. See Cache.
            lock (Cache)
            {
                if (!Cache.ContainsKey(sys))
                {
                    Cache[sys] = sys.ActorOf(Props.Create<NetworkActor>().WithDispatcher("calling-thread-dispatcher"));
                }
                return new Network(Cache[sys]);
            }
        }
    }
    
    public class NetworkActor : UntypedActor
    {
        public class CreateOmnetNode
        {
            public readonly IActorRef Node;

            public CreateOmnetNode(IActorRef node)
            {
                Node = node;
            }
        }
        
        public class NetworkMessage
        {
            public readonly IActorRef Sender;
            public readonly object Message;

            public NetworkMessage(IActorRef sender, object message)
            {
                Sender = sender;
                Message = message;
            }
        }
        
        public class Send : NetworkMessage
        {
            public readonly IActorRef Receiver;

            public Send(IActorRef sender, IActorRef receiver, object message) : base(sender, message)
            {
                Receiver = receiver;
            }
        }
        
        public class Broadcast : NetworkMessage
        {
            public Broadcast(IActorRef sender, object message) : base(sender, message)
            {
            }
        }
        
        public class Subscribe
        {
            public readonly Type Topic;
            public readonly IActorRef Subscriber;

            public Subscribe(IActorRef subscriber, Type topic)
            {
                Subscriber = subscriber;
                Topic = topic;
            }
        }

        private ILoggingAdapter _log = Context.System.Log;
        private ImmutableDictionary<ulong, IActorRef> _idToRef = ImmutableDictionary<ulong, IActorRef>.Empty;
        private ImmutableDictionary<IActorRef, ulong> _refToId = ImmutableDictionary<IActorRef, ulong>.Empty;
        private int _nextMessageId;
        /// <summary>
        /// The key is a tuple of MessageId and ReceiverId. After a broadcast we need to distinguish different receivers, because message are delivered at different points in time.
        /// </summary>
        private readonly ConcurrentDictionary<Tuple<int, ulong>, Task> _waitingForOmnetToDelvierMessage = new ConcurrentDictionary<Tuple<int, ulong>, Task>();
        private IImmutableDictionary<Type, IImmutableList<IActorRef>> _subscribers = ImmutableDictionary<Type, IImmutableList<IActorRef>>.Empty;

        public NetworkActor()
        {
        }

        public override void AroundPreStart()
        {
            base.AroundPreStart();
            OmnetInterface.Instance.OMNeTReceive += OmnetOnOmneTReceive;
        }

        public override void AroundPostStop()
        {
            base.AroundPostStop();
            OmnetInterface.Instance.OMNeTReceive -= OmnetOnOmneTReceive;
        }

        protected override void OnReceive(object message)
        {
            if (message is CreateOmnetNode create)
            {
                var actorRef = create.Node;
                var id = OmnetSimulation.Instance().CreateNodeAndId();
                _idToRef = _idToRef.Add(id, actorRef);
                _refToId = _refToId.Add(actorRef, id);
            }
            else if (message is Subscribe subscribe)
            {
                var topic = subscribe.Topic;
                if (!_subscribers.ContainsKey(topic))
                {
                    _subscribers = _subscribers.Add(topic, ImmutableList<IActorRef>.Empty);
                }
                _subscribers = _subscribers.SetItem(topic, _subscribers[topic].Add(subscribe.Subscriber));
            }
            else if (message is Broadcast broadcast)
            {
                // There seems to be no way to use publish here... we need to implement pubsub manually.
                var sender = broadcast.Sender;
                var broadcasted = broadcast.Message;
                var topic = broadcasted.GetType();
                var msgId = _nextMessageId++;
                if (_subscribers.ContainsKey(topic))
                {
                    _subscribers[topic].ForEach(sub =>
                    {
                        if (_refToId.TryGetValue(sub, out var subId))
                        {
                            var task = new Task(() =>
                            {
                                sub.Tell(broadcasted, sender);
                            });
                            _waitingForOmnetToDelvierMessage.TryAdd(Tuple.Create(msgId, subId), task);
                        }
                    });
                }
                else
                {
                    Context.System.Log.Warning("No subscribers for topic " + topic.FullName);
                }
                BroadcastInOmnet(sender, broadcasted, msgId);
            }
            else if (message is Send send)
            {
                SendInOmnet(send.Sender, send.Receiver, send.Message)
                    .ContinueWith(result => { send.Receiver.Tell(send.Message, send.Sender); });
            }
        }

        public void BroadcastInOmnet(IActorRef sender, object message, int msgId)
        {
            _log.Info("Broadcasting {0} from {1} ", message.GetType(), sender?.Path.Name);
            if (_refToId.TryGetValue(sender, out var senderId))
            {
                _log.Info($"Broadcasting {msgId} to omnet.");
                var numberOfBytes = 10;
                OmnetSimulation.Instance().Send(senderId, OmnetSimulation.BROADCAST_ADDR, numberOfBytes, msgId);
            }
            else
            {
                throw new Exception("Key for sender not found");
            }
        }
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="receiver"></param>
        /// <param name="message"></param>
        /// <returns></returns>
        public Task SendInOmnet(IActorRef sender, IActorRef receiver, object message)
        {
            Console.WriteLine("Sending {0} from {1} to {2}", message.GetType(), sender?.Path.Name, receiver?.Path.Name);
            var task = new Task(() => { });
            if (
                _refToId.TryGetValue(sender, out var senderId) 
                && 
                _refToId.TryGetValue(receiver, out var receiverId))
            {
                var msgId = _nextMessageId++;
                _log.Info($"Sending message {msgId} to omnet.");
                // Add to the dictionary first, since omnet might invoce the callback immediately.
                _waitingForOmnetToDelvierMessage.TryAdd(Tuple.Create(msgId, receiverId), task);
                var numberOfBytes = 10;
                Console.WriteLine("Sending message to omnet. Sender:"+senderId+" Receiver:"+receiverId+" MsgId:"+msgId+"Bytes:"+numberOfBytes);
                OmnetSimulation.Instance().Send(senderId, receiverId, numberOfBytes, msgId);
            }
            else
            {
                throw new Exception("Key for sender or receiver not found");
            }
            return task;
        }
        
        private void OmnetOnOmneTReceive(ulong dest, ulong source, int id, int statusFlag)
        {
            // TODO: Handle not delivered.
            _log.Info($"Handled omnet receive for msg {id} and continue to deliver message from {source} to {dest}.");
            if (_waitingForOmnetToDelvierMessage.TryRemove(Tuple.Create(id, dest), out var task))
            {
                task.Start();
            }
        }
    }
}