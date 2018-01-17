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
		
		
		
        public static void Main(string[] args) 
        {
            Console.WriteLine("Hello from C#");
        }
        
        // method invoked by omnet++
        static void initSimulation()
        {
        	aa_createNode(111);
            aa_createNode(222);
            
            //ulong id = 0;
            //for (int i = 1; i <= 1000; i++)
            //{
            //	aa_createNode(id);
            //	id++;
            //}
            
            //ulong id = aa_createNodeAndId();
            //Console.WriteLine("received Id is {0}", id);
        }
    }
}
