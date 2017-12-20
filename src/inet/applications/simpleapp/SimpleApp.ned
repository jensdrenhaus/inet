//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.applications.simpleapp;

import inet.applications.contract.ISimpleApp;

//
// TODO 
//
simple SimpleApp like ISimpleApp
{
    parameters:
        string destAddr = default(""); // destination MAC address(es), separated by spaces 
        string srcAddr = default(""); // source address 
        double packetSize @unit("B") = default(56B); // of ping payload, in bytes
        volatile double sendInterval @unit("s") = default(1s); // time to wait between pings (can be random)
        double count = default(-1); // number of pings requests sent to a single destination address, -1 means continuously (only first address will be used from destAddr)
        double startTime @unit("s") = default(uniform(0s, this.sleepDuration + this.sendInterval)); // send first ping at startTime
        double stopTime @unit("s") = default(-1s); // time to finish sending, negative values mean forever
        volatile double sleepDuration @unit(s) = default(0s); // time spent in sleep between switching destinations
        bool continuous = default(false);  // whether to continuously ping the destinations in a round-robin fashion
        bool printPing = default(false); // log to stdout
        @display("i=block/app");
        @signal[rtt](type=simtime_t);
        @signal[numLost](type=long);
        @signal[numOutOfOrderArrivals](type=long);
        @signal[pingTxSeq](type=long);
        @signal[pingRxSeq](type=long);
        @statistic[rtt](title="ping round-trip time"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[numLost](title="pings lost"; record=last,vector; interpolationmode=none);
        @statistic[numOutOfOrderArrivals](title="ping out-of-order arrivals"; record=last,vector; interpolationmode=none);
        @statistic[pingTxSeq](title="ping tx seq"; record=count,vector; interpolationmode=none);
        @statistic[pingRxSeq](title="ping rx seq"; record=count,vector; interpolationmode=none);
    gates:
	    input appIn @labels(AppPayload/up);
	    output appOut @labels(AppPayload/down);
}
