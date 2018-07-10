using System;
using System.Runtime.InteropServices;
using NativeLibraryLoader;

namespace OmnetServices
{
	public class OmnetSimulation
	{
		private delegate void TestType(string text);
		private TestType _Test = null;

		private delegate ulong CreateNodeAndIdType();
		private CreateNodeAndIdType _CreateNodeAndId = null;
        
		private delegate void CreateNodeType(ulong id);
		private CreateNodeType _CreateNode = null;

		private delegate void SendType(ulong srcId, ulong destId, int numBytes, int msgId);
		private SendType _Send = null;

		private delegate void WaitMillisecondsType(ulong id, int duration);
		private WaitMillisecondsType _WaitMilliseconds = null;

		private delegate void WaitSecondsType(ulong id, int duration);
		private WaitSecondsType _WaitSeconds = null;

		private delegate void SetGlobalTimerMillisecoundsType(int duration);
		private SetGlobalTimerMillisecoundsType _SetGlobalTimerMillisecounds = null;

		private delegate void SetGlobalTimerSecondsType(int duration);
		private SetGlobalTimerSecondsType _SetGlobalTimerSeconds = null;
        
		private delegate ulong GetGlobalTimeType();
		private GetGlobalTimeType _GetGlobalTime = null;

		private static readonly OmnetSimulation instance = new OmnetSimulation();

		public static OmnetSimulation Instance()
		{
			return instance;
		}

		private OmnetSimulation()
		{
			NativeLibrary runtimehost = new NativeLibrary(null);
			_Test = runtimehost.LoadFunction<TestType>("test");
			_CreateNodeAndId = runtimehost.LoadFunction<CreateNodeAndIdType>("daa_createNodeAndId");
			_CreateNode = runtimehost.LoadFunction<CreateNodeType>("daa_createNode");
			_Send = runtimehost.LoadFunction<SendType>("daa_send");
			_WaitMilliseconds = runtimehost.LoadFunction<WaitMillisecondsType>("daa_wait_ms");
			_WaitSeconds = runtimehost.LoadFunction<WaitSecondsType>("daa_wait_s");
			_SetGlobalTimerMillisecounds = runtimehost.LoadFunction<SetGlobalTimerMillisecoundsType>("daa_set_global_timer_ms");
			_SetGlobalTimerSeconds = runtimehost.LoadFunction<SetGlobalTimerSecondsType>("daa_set_global_timer_s");
			_GetGlobalTime = runtimehost.LoadFunction<GetGlobalTimeType>("daa_getGlobalTime");
		}

		public void Test()
		{
			_Test("C# is calling back into the runtime");
		}

		public ulong CreateNodeAndId()
        {
			return _CreateNodeAndId();
        }
        
		public void CreateNode(ulong id)
        {
			_CreateNode(id);
        }

		public void Send(ulong srcId, ulong destId, int numBytes, int msgId)
        {
			_Send(srcId, destId, numBytes, msgId);
        }
        
		public void WaitMilliseconds(ulong id, int duration)
        {
			_WaitMilliseconds(id, duration);
        }

		public void WaitSeconds(ulong id, int duration)
        {
            _WaitSeconds(id, duration);
        }

		public void SetGlobalTimerMillisecounds(int duration)
        {
			_SetGlobalTimerMillisecounds(duration);
        }

		public void SetGlobalTimerSeconds(int duration)
        {
			_SetGlobalTimerSeconds(duration);
        }

		public ulong GetGlobalTime() // in picoseconds ( 1ps = 1*10E-18s )
		{
			return _GetGlobalTime();
		}
       
	}
}