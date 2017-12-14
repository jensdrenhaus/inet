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

package inet.linklayer.ideal;

import inet.linklayer.base.MACProtocolBase;
import inet.linklayer.contract.ILinkLayer;
import inet.linklayer.contract.IMACProtocol;


// Like IdealMac, but using SimpleLinkLayerControllInfo instead of Ieee802Ctrl
//
// Implements a trivial MAC layer for use in ~IdealWirelessNic.
//
// This module only provides encapsulation/decapsulation (into IdealMacFrame),
// but no real medium access protocol.
//
// Higher layer packets should have SimpleLinkLayerControllInfo attached to them as
// control info.
//
// See ~IdealWirelessNic for rationale and details.
//
simple SimpleMac extends MACProtocolBase like ILinkLayer, IMACProtocol
{
    parameters:
        string address = default("auto");   // MAC address as hex string (12 hex digits), or
                                            // "auto". "auto" values will be replaced by
                                            // a generated MAC address in init stage 0.
        double bitrate @unit("bps");
        int mtu @unit("B") = default(4470B);
        int headerLength @unit("B") = default(0B);       // IdealMacFrame.byteLength = headerLength + packetFromUpperLayer.byteLength
        bool promiscuous = default(false);
        bool fullDuplex = default(true);    // allows transmitting and receiving simultaneously (transceiver radio mode)
        bool useAck = default(true);
        double ackTimeout @unit("s") = default(100ms);

        @class(IdealMac);
        @signal[dropPkNotForUs](type=IdealMacFrame);
        @signal[NF_LINK_BREAK](type=IdealMacFrame);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDownPk](title="packets sent to lower layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHL](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromLL](title="packets received from lower layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkNotForUs](title="packets dropped/not for us"; source=dropPkNotForUs; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
}
