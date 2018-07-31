using System;

namespace OmnetServices
{
	// Paste your code into the respective callback functions.
	// You can use the OmnetSimulation class to

	// Example:
    // This example creates two nodes. When the simulation starts node1 sends a message to node2
	// an the global timer is set to 2 seconds. when the timer has expired, node1 sends again 
	// and the timer is again set to 2 seconds .. never ending story ;)
	// For the incoming events, the current simulation time is output.

    public class OmnetInterface
    {

        public static void initSimulation()
        {
            Console.WriteLine("C# : initSimulation is called");

            //Example

            OmnetSimulation.Instance().CreateNode(1);
            OmnetSimulation.Instance().CreateNode(2);
			OmnetSimulation.Instance().CreateNode(3);
			OmnetSimulation.Instance().CreateNode(4);
			OmnetSimulation.Instance().CreateNode(5);
			OmnetSimulation.Instance().CreateNode(6);
			OmnetSimulation.Instance().CreateNode(7);
			OmnetSimulation.Instance().CreateNode(8);
			OmnetSimulation.Instance().CreateNode(9);
			OmnetSimulation.Instance().CreateNode(10);

            OmnetSimulation.Instance().CreateNode(11);
			OmnetSimulation.Instance().CreateNode(12);
			OmnetSimulation.Instance().CreateNode(13);
			OmnetSimulation.Instance().CreateNode(14);
			OmnetSimulation.Instance().CreateNode(15);
			OmnetSimulation.Instance().CreateNode(16);
			OmnetSimulation.Instance().CreateNode(17);
			OmnetSimulation.Instance().CreateNode(18);
			OmnetSimulation.Instance().CreateNode(19);
			OmnetSimulation.Instance().CreateNode(20);
			

            OmnetSimulation.Instance().GetGlobalTime();
        }

		public static void simulationReady()
        {
            Console.WriteLine("C# : simulationReady is called");

			//Example
            OmnetSimulation.Instance().Send(1, 0x0000ffffffffffff, 10, 123);
            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
            OmnetSimulation.Instance().SetGlobalTimerSeconds(2);
        }

		public static void simulationFinished()
        {
            Console.WriteLine("C# : simulationFinished is called");

			//Example
            //...
        }

		public static void receptionNotify(ulong destId, ulong srcId, int msgId, int status)
        {
            Console.WriteLine("C# : receptionNotify is called with destID=" + destId + " srcId=" + srcId + " msgId=" + msgId + " status=" + status);

			//Example
            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
			if (destId == 20){
				OmnetSimulation.Instance().Send(20, srcId, 10, 123);
			}
			if (destId == 10){
				OmnetSimulation.Instance().Send(10, srcId, 10, 123);
			}

        }

		public static void timerNotify(ulong nodeId)
        {
            Console.WriteLine("C# : timerNotify is called with nodeId=" + nodeId);

			//Example
            //...
        }

		public static void globalTimerNotify()
        {
            Console.WriteLine("C# : globalTimerNotify is called");

			//Example
            OmnetSimulation.Instance().Send(1, 0x0000ffffffffffff, 10, 123);
            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
            OmnetSimulation.Instance().SetGlobalTimerSeconds(2);
        }

    }
}