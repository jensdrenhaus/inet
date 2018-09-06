using System;
using Akka.Actor;
using SampleApp;

namespace PhyNetFlow.OMNeT
{
    public class EchoBroadcaster : UntypedActor, ILogReceive
    {
        private readonly INetwork _network;

        public EchoBroadcaster()
        {
            _network = Context.System.GetNetwork();
        }

        protected override void OnReceive(object message)
        {
            Console.WriteLine("Received a message. " + Self.Path);
            if (message is SendEcho)
            {
                _network.Broadcast(Self, new Echo());
                // After 1 second broadcast new echo message.
                Context.System.Scheduler.ScheduleTellOnce(TimeSpan.FromMilliseconds(1000), Self, new SendEcho(), Self);
            }        
        }
    }
    
    public class EchoActor : UntypedActor, ILogReceive
    {
        private readonly bool _shouldIgnore;
        private readonly INetwork _network;

        public EchoActor(bool shouldIgnore)
        {
            _shouldIgnore = shouldIgnore;
            _network = Context.System.GetNetwork();
            _network.Subscribe(Self, typeof(Echo));
        }

        protected override void OnReceive(object message)
        {
            Console.WriteLine("Received a message. " + Self.Path);
            if (message is Echo && !_shouldIgnore)
            {
                _network.Send(Self, Sender, message);
            }
        }

        public static Props Props(bool shouldIgnore = false)
        {
            return Akka.Actor.Props.Create(() => new EchoActor(shouldIgnore));
        }
    }

    public class SendEcho
    {
    }

    /// <summary>
    /// Message to be send.
    /// </summary>
    public class Echo
    {
    }
}