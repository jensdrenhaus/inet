using System;
using Akka.Actor;

namespace PhyNetFlow.OMNeT
{
    public class EchoActor : UntypedActor, ILogReceive
    {
        public class Echo
        {
        }

        protected override void OnReceive(object message)
        {
            Console.WriteLine("Received a message.");
            // After 1 second reply with a new echo message.
            Context.System.Scheduler.ScheduleTellOnce(TimeSpan.FromMilliseconds(1000), Sender, new Echo(), Self);
        }

        public static Props Props()
        {
            return Akka.Actor.Props.Create(() => new EchoActor());
        }
    }
}