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

            OmnetSimulation.Instance().CreateNode(1, OmnetSimulation.ACCESSPOINT_TYPE);
            OmnetSimulation.Instance().CreateNode(2, OmnetSimulation.PHYNODE_IGNORING_TYPE);
			OmnetSimulation.Instance().CreateNode(3, OmnetSimulation.PHYNODE_IGNORING_TYPE);
			OmnetSimulation.Instance().CreateNode(4, OmnetSimulation.PHYNODE_IGNORING_TYPE);
			OmnetSimulation.Instance().CreateNode(5, OmnetSimulation.PHYNODE_IGNORING_TYPE);
			OmnetSimulation.Instance().CreateNode(6, OmnetSimulation.PHYNODE_IGNORING_TYPE);
			OmnetSimulation.Instance().CreateNode(7, OmnetSimulation.PHYNODE_RESPONDING_TYPE);
			OmnetSimulation.Instance().CreateNode(8, OmnetSimulation.PHYNODE_RESPONDING_TYPE);
			OmnetSimulation.Instance().CreateNode(9, OmnetSimulation.PHYNODE_RESPONDING_TYPE);
			OmnetSimulation.Instance().CreateNode(10, OmnetSimulation.PHYNODE_RESPONDING_TYPE);
			OmnetSimulation.Instance().CreateNode(11, OmnetSimulation.PHYNODE_RESPONDING_TYPE);
			
            OmnetSimulation.Instance().GetGlobalTime();
        }

		public static void simulationReady()
        {
            Console.WriteLine("C# : simulationReady is called");

			//Example
            OmnetSimulation.Instance().Send(1, OmnetSimulation.BROADCAST_ADDR, 10, 123);
            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
            OmnetSimulation.Instance().SetGlobalTimerSeconds(5);
        }

		public static void simulationFinished()
        {
            Console.WriteLine("C# : simulationFinished is called");

			//Example
            //...
        }

		public static void receptionNotify(ulong destId, ulong srcId, int msgId, int status)
        {
			string statusString = status==OmnetSimulation.RECEPTION_OK ? "OK" : status==OmnetSimulation.RECEPTION_IGNORED ? "IGNORED" : status==OmnetSimulation.RECEPTION_BITERROR ? "BITERROR" : "UNKNOWN";
            Console.WriteLine("C# : receptionNotify is called with destID=" + destId + " srcId=" + srcId + " msgId=" + msgId + " status=" + statusString);

			//Example
            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
			if (status == OmnetSimulation.RECEPTION_OK){
				if (destId == 7){
					OmnetSimulation.Instance().Send(7, srcId, 10, 123);
				}
				if (destId == 8){
					OmnetSimulation.Instance().Send(8, srcId, 10, 123);
				}
				if (destId == 9){
					OmnetSimulation.Instance().Send(9, srcId, 10, 123);
				}
				if (destId == 10){
					OmnetSimulation.Instance().Send(10, srcId, 10, 123);
				}
				if (destId == 11){
					OmnetSimulation.Instance().Send(11, srcId, 10, 123);
				}
			}
        }

		public static void timerNotify(ulong nodeId)
        {
            Console.WriteLine("C# : timerNotify is called with nodeId=" + nodeId);

			//Example
            //...
        }
		
		private static int cnt = 0;
		
		public static void globalTimerNotify()
        {
			cnt++;
            Console.WriteLine("C# : globalTimerNotify is called");
            Console.WriteLine("Count: " + cnt);
            if(cnt < 100) {
            	OmnetSimulation.Instance().Send(1, OmnetSimulation.BROADCAST_ADDR, 10, 123);
	            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
	            OmnetSimulation.Instance().SetGlobalTimerSeconds(5);
				Console.WriteLine("Count: " + cnt);
            }
        }

    }
}