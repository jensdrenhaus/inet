// A Hello World! program in C#.

using System;

//for mono runtime P/Invoke system (calling C functions)
using System.Runtime.InteropServices;

namespace CSProgram
{
    class Program 
    {
    	//for mono runtime P/Invoke system (calling C functions)
    	
		[DllImport ("__Internal")]
		extern static ulong aa_createNodeAndId();
		
		[DllImport ("__Internal")]
		extern static void aa_createNode(ulong id);
		
		[DllImport ("__Internal")]
		extern static void aa_send(ulong from_id);
		
		[DllImport ("__Internal")]
		extern static void aa_wait_ms(ulong id, int duration);
		
		[DllImport ("__Internal")]
		extern static void aa_wait_s(ulong id, int duration);
		
		
		// NO INFINITY LOOP IN HERE !!! 
        public static void Main(string[] args) 
        {
            Console.WriteLine("C# : Hello!");
        }
        
        // method invoked by omnet++ before simulation starts
        static void initSimulation()
        {
        	aa_createNode(111);
            aa_createNode(222);
            aa_createNode(333);
            
            //ulong id = 0;
            //for (int i = 1; i <= 1000; i++)
            //{
            //	aa_createNode(id);
            //	id++;
            //}
            
            //ulong id = aa_createNodeAndId();
            //Console.WriteLine("received Id is {0}", id);
        }
        
        //method invoked by omnet++ at simulation start
        static void simulationReady()
        {
        	//for (int i = 0; i < 3; i++)
        	//{
        	//	Console.WriteLine("C# : send message from Node with Id 111");
        	//	aa_send(111);
        	//}
        	//aa_wait_s(111,2);
        	aa_send(111);
        }
        
        //method invoked by omnet++ at reception events
        static void receptionNotify(ulong nodeId)
        {
        	Console.WriteLine("C# : got reception notification from {0}", nodeId);
        	if (nodeId != 111) {
        		Console.WriteLine("C# : send echo");
        		aa_send(nodeId);
        	}
        }
        
        //method invoked by omnet++ at reception events
        static void timerNotify(ulong nodeID)
        {
        	Console.WriteLine("C# : got timer notification from {0}", nodeID);
        	for (int i = 0; i < 3; i++)
        	{
        		Console.WriteLine("C# : send message from Node with Id 111");
        		aa_send(111);
        	}
        }
    }
}
