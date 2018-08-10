using System;
using System.Collections.Concurrent;
using System.Collections.Immutable;
using System.Threading.Tasks;
using Akka.Actor;
using OmnetServices;
using PhyNetFlow.Core.Actors;
using PhyNetFlow.Core.Akka;
using SampleApp;

// ReSharper disable once CheckNamespace
namespace PhyNetFlow.OMNeT
{

    /**
     *
     *
     *
     * IMPORTANT:
     * Because there was no easy way to implement broadcasts, we now use the Network.
     * The new scenario creates 3 actors, one that broadcasts, one that replies and one that does nothing.
     * The plugin should only setup now, but not handle messages in omnet anymore. This is now done within the network.
     *
     * 
     */
    public class Plugin : FSM<Plugin.State, Plugin.Data>, ILogReceive, ISimulationMessageDelegate
    {
        // ReSharper disable once InconsistentNaming
        private class OMNeTReady
        {
        }

        // ReSharper disable once InconsistentNaming
        private class OMNeTStared
        {

        }

        public enum State
        {
            Uninitialized,
            // ReSharper disable once InconsistentNaming
            WaitingForOMNet,
            WaitingForPhyNet,
            WaitingForSimulationStart,
            Running
        }

        public class Data
        {
            public static readonly Data Empty = new Data();
        }

        private readonly OmnetInterface _omnet;
        private readonly IActorRef _selfRef;
        private ImmutableDictionary<ulong, IActorRef> _idToRef;
        private ImmutableDictionary<IActorRef, ulong> _refToId;
        private IActorRef _echoBroadcaster;
        private IActorRef _echoReplier;
        private INetwork _network;

        public Plugin()
        {
            _network = Context.System.GetNetwork();
            Context.System.Log.Info("Started " + typeof(Plugin).Name);

            // Configure self as the delegate.
            if (Context.System is ExtendedActorSystem system && system.Provider is OmnetActorRefProvider provider)
            {
                provider.SimulationMessageDelegate = this;
            }

            // Save a reference to be able to send messages from within OMNeT calls.
            _selfRef = Self;

            // Create a dictionary that maps OMNeT ids to IActorRefs.
            _idToRef = ImmutableDictionary<ulong, IActorRef>.Empty;
            _refToId = ImmutableDictionary<IActorRef, ulong>.Empty;

            // Register OMNeT events.
            _omnet = OmnetInterface.Instance;
            _omnet.OMNeTBeforeStart += OmnetOnOmneTBeforeStart;
            _omnet.OMNeTReceive += OmnetOnOmneTReceive;
            _omnet.OMNeTStart += OmnetOnOmneTStart;

            // Subscribe to ready events.
            Context.System.EventStream.Subscribe(Self, typeof(PhyNetContainer.Ready));

            //
            // Define the state machine.
            //
            StartWith(State.Uninitialized, Data.Empty);

            // TODO On simulationReady -> create one actor and create a broadcast message.

            // We need to wait for two environments to be ready, PhyNet and OmneT
            When(State.Uninitialized, e =>
            {
                if (e.FsmEvent is PhyNetContainer.Ready)
                {
                    return GoTo(State.WaitingForOMNet);
                }

                if (e.FsmEvent is OMNeTReady)
                {
                    return GoTo(State.WaitingForPhyNet);
                }

                return null;
            });

            // PhyNet is ready, wait for omnet++ init
            When(State.WaitingForOMNet, e =>
            {
                if (e.FsmEvent is OMNeTReady)
                {
                    return GoTo(State.WaitingForSimulationStart);
                }
                return null;
            });

            // omnet++ is ready, waiting for PHyNet
            When(State.WaitingForPhyNet, e =>
            {
                if (e.FsmEvent is PhyNetContainer.Ready)
                {
                    return GoTo(State.WaitingForSimulationStart);
                }

                return null;
            });

            // This should be called after PhyNetContainer is ready, during the omnet++ initialization phase.
            // Since everything should be scheduled to execute synchronously, the Actors/Nodes will be created
            // while in the init method.
            // After this we wait for the omnet simulation to start.
            OnTransition((state, nextState) =>
            {
                if (nextState == State.WaitingForSimulationStart)
                {
                    Console.WriteLine("Creating actors/nodes in omnet and phynetflow.");
                    // Setup nodes/agents for omnet++ and phynetflow.
                    // One that broadcasts, one that responds with echo, one that does nothing.
                    _echoBroadcaster = CreateOMNeTActor(Props.Create<EchoBroadcaster>().WithDispatcher("calling-thread-dispatcher"));
                    _echoReplier = CreateOMNeTActor(EchoActor.Props().WithDispatcher("calling-thread-dispatcher"));
                    _echoIgnorer = CreateOMNeTActor(EchoActor.Props(true).WithDispatcher("calling-thread-dispatcher"));
                }
            });

            When(State.WaitingForSimulationStart, e =>
            {
                if (e.FsmEvent is OMNeTStared)
                {
                    return GoTo(State.Running);
                }

                return null;
            });

            When(State.Running, e => null);

            // When transitioning to Running, create OmneT instances.
            OnTransition((state, nextState) =>
            {
                if (nextState == State.Running)
                {
                    Console.WriteLine("Setting state to running, sending message.");
                    _echoBroadcaster.Tell(new SendEcho(), _echoBroadcaster);
                }
            });

            WhenUnhandled(e =>
            {
                if (e.FsmEvent is PhyNetContainer.StartConfiguration)
                {
                    return Stay().Replying(new PhyNetContainer.PluginConfiguredNotification(GetType()));
                }

                return null;
            });

            OnTransition((state, nextState) =>
            {
                Console.WriteLine("Transitioning from " + state + " to " + nextState);
            });
        }

        /// <summary>
        /// Creates an actor in the actorsystem and in omnet++.
        /// </summary>
        /// <param name="props"></param>
        /// <returns></returns>
        // ReSharper disable once InconsistentNaming
        private IActorRef CreateOMNeTActor(Props props)
        {
            var actorRef = Context.ActorOf(props);
            (_network as Network)?.CreateOmnetNode(actorRef);
            return actorRef;
            /*
                var id = OmnetSimulation.Instance().CreateNodeAndId();
    
                _idToRef = _idToRef.Add(id, actorRef);
                _refToId = _refToId.Add(actorRef, id);
    
                return actorRef;
            */
        } 

        private void OmnetOnOmneTStart()
        {
            _selfRef.Tell(new OMNeTStared());
        }

        private void OmnetOnOmneTReceive(ulong dest, ulong source, int id, int statusFlag)
        {
            Console.WriteLine("Handled omnet receive and continue to deliver message to actor.");
            if (_waitingForOmnetToDelvierMessage.TryRemove(id, out var task))
            {
                task.Start();
            }
        }

        private void OmnetOnOmneTBeforeStart()
        {
            _selfRef.Tell(new OMNeTReady());
        }

        public override void AroundPostStop()
        {
            base.AroundPostStop();

            _omnet.OMNeTBeforeStart -= OmnetOnOmneTBeforeStart;
            _omnet.OMNeTStart -= OmnetOnOmneTStart;
            _omnet.OMNeTReceive -= OmnetOnOmneTReceive;

            if (Context.System is ExtendedActorSystem system && system.Provider is SimulationActorRefProvider provider)
            {
                provider.SimulationMessageDelegate = null;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="receiver"></param>
        /// <param name="message"></param>
        /// <returns></returns>
        public Task DelayMessageDelivery(IActorRef sender, IActorRef receiver, object message)
        {
            /*
            Console.WriteLine("Sending {0} from {1} to {2}", message.GetType(), sender?.Path.Name, receiver?.Path.Name);
            var task = new Task(() => { });
            if (_refToId.TryGetValue(sender, out var senderId) && _refToId.TryGetValue(receiver, out var receiverId))
            {
                Console.WriteLine("Sending message to omnet.");
                var msgId = _nextMessageId++;
                // Add to the dictionary first, since omnet might invoce the callback immediately.
                _waitingForOmnetToDelvierMessage.TryAdd(msgId, task);
                // TODO: Number of bytes!
                var numberOfBytes = 10;
                OmnetSimulation.Instance().Send(senderId, receiverId, numberOfBytes, msgId);
            }

            return task;
            */
            
            // TODO: !! USES NETWORK
            return Task.CompletedTask;
        }

        private int _nextMessageId;
        private readonly ConcurrentDictionary<int, Task> _waitingForOmnetToDelvierMessage = new ConcurrentDictionary<int, Task>();
        private IActorRef _echoIgnorer;
    }
}