using System;
using System.ComponentModel;
using Akka.Actor;
using Akka.Configuration;
using PhyNetFlow.Core.Actors;
using PhyNetFlow.OMNeT;

// ReSharper disable InconsistentNaming

// ReSharper disable once CheckNamespace
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
	    public ulong TimeAtStart { get; set; }
	    public DateTime DateTimeOffsetAtStart { get; set; }
	    
	    /// <summary>
	    ///     Creates the configured actor system.
	    /// </summary>
	    private static void CreateConfiguredActorSystem()
	    {
		    var configString = $@"
                akka {{
                    loglevel = DEBUG,
                    # loggers = [""Akka.Logger.Serilog.SerilogLogger, Akka.Logger.Serilog""],
                    log-config-on-start = off,
                    actor {{
                        # default-dispatcher.executor = ""current-context-executor""
                        # default-dispatcher = ""calling-thread-dispatcher""
					
						# The provider forces the usage of calling thread dispatcher, this way all code is executed on
						# the main thread.
                        provider = ""{typeof(OmnetActorRefProvider).AssemblyQualifiedName}""
                        debug {{
                            receive = on,
                            autoreceive = off,
                            lifecycle = off,
                            event-stream = off,
                            unhandled = off
                        }}
                    }}
					# This scheduler usees omnet for scheduling
                    scheduler {{ implementation = ""{typeof(OmnetScheduler).AssemblyQualifiedName}"" }}
                }}
                ";
		    var configParsed = ConfigurationFactory.ParseString(configString);
		    Sys = ActorSystem.Create("sys", configParsed);
	    }
	    
	    protected virtual void OnOmneTBeforeStart()
	    {
		    OMNeTBeforeStart?.Invoke();
	    }

	    public delegate void OnBeforeOMNeTStart();
	    public event OnBeforeOMNeTStart OMNeTBeforeStart;
	    
        public static void initSimulation()
        {
            Console.WriteLine("C# : initSimulation is called");

	        Instance.TimeAtStart = OmnetSimulation.Instance().GetGlobalTime();
	        Instance.DateTimeOffsetAtStart = DateTime.Now;
	        
            //Example
            // OmnetSimulation.Instance().CreateNode(1);
            // OmnetSimulation.Instance().CreateNode(2);
            // OmnetSimulation.Instance().GetGlobalTime();
	        // Context = new BulkExecutingSynchronizationContext();
	        // SynchronizationContext.SetSynchronizationContext(Context);
	        
	        Console.WriteLine("Received init from omnet++.");
	        // Create and configure the actor system.
	        CreateConfiguredActorSystem();
	        // Context.EnterControlledExecutionMode();

	        Console.WriteLine("Created actor system.");

	        // Create and configure the PhyNet runtime.
	        // This loads plugins etc.
	        var approot = Sys.ActorOf(PhyNetContainer.Props.WithDispatcher("calling-thread-dispatcher"), "approot");
	        var config = PhyNetContainer.GetConfigFromString("{\r\n\t\"plugins\": [\r\n\t\t\"" + typeof(Plugin).AssemblyQualifiedName + "\"\r\n\t] \r\n}");
	        approot.Tell(new PhyNetContainer.Configure(config));

	        // Setup the PhyNet and the plugin.
	        // We need to do this before actually triggering the event, because otherwise no listener will exist.
	        // TODO Context.ExecuteAll();
	        Console.WriteLine("Told Container to configure.");

	        // Actually trigger the event.
	        Instance.OnOmneTBeforeStart();
	        // TODO Context.ExecuteAll();
	        Console.WriteLine("Done with init.");
	        // Here everything is ready and we can wait for omnet++.
        }

	    public static ActorSystem Sys { get; set; }
	    public static readonly OmnetInterface Instance = new OmnetInterface();


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
            // OmnetSimulation.Instance().Send(1, 2, 10, 123);
            Console.WriteLine("Time: " + OmnetSimulation.Instance().GetGlobalTime() + "ps");
            // OmnetSimulation.Instance().SetGlobalTimerSeconds(2);
	        Instance.OmnetGlobalTimerNotify?.Invoke();
        }

	    public delegate void OnOmnetGlobalTimerNotify();

	    public event OnOmnetGlobalTimerNotify OmnetGlobalTimerNotify;
	    
	    #region Start
	    protected virtual void OnOmneTStart()
	    {
		    OMNeTStart?.Invoke();
	    }

	    public delegate void OnOMNeTStart();

	    public event OnOMNeTStart OMNeTStart;

	    // method invoked by omnet++ at simulation start
	    public static void simulationReady()
	    {
		    Console.WriteLine("Received simulation ready from omnet++.");
		    Instance.OnOmneTStart();
		    // TODO Context.ExecuteAll();
	    }
	    #endregion

	    #region Receiving
	    protected virtual void OnOmneTReceive(ulong dest, ulong src, int id, int statusFlag)
	    {
		    OMNeTReceive?.Invoke(dest, src, id, statusFlag);
	    }

	    public delegate void OnOMNeTReceive(ulong dest, ulong source, int id, int statusFlag);

	    public event OnOMNeTReceive OMNeTReceive;
	    // method invoked by omnet++ at reception events
	    public static void receptionNotify(ulong dest, ulong source, int id, int statusFlag)
	    {
		    Console.WriteLine("Received reception notify from omnet++.");
		    Instance.OnOmneTReceive(dest, source, id, statusFlag);
		    // TODO Context.ExecuteAll();
	    }
	    #endregion
	    #region Shutdown
	    // method invoed by omnet++ when simulation finished
	    public static void simulationFinished()
	    {
		    // TODO Context.ExecuteAll();
		    Sys.Terminate();
		    // TODO Context.ExecuteAll();
	    }
	    #endregion

    }
}