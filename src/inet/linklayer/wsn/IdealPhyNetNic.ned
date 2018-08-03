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

package inet.linklayer.wsn;

import inet.linklayer.wsn.PhyNetNicBase;


// Highly abstracted wireless NIC that consists of a unit disk radio and a trivial
// MAC protocol. It offers simplicity for scenarios where Layer 1 and 2 effects
// can be completely ignored, for example testing the basic functionality
// of a wireless ad-hoc routing protocol.
//
// The most important parameter this model accepts is the transmission range.
// When a radio transmits a frame, all other radios within transmission range
// will receive the frame correctly, and radios out of range will not be
// affected at all.
//
// This module requires an ~IdealRadioMedium instance in the network.
//
module IdealPhyNetNic extends PhyNetNicBase
{
    parameters:
        queueType = default("DropTailQueue");    // DropTailQueue, a Diffserv queue, etc.
        macType = default("SimpleMac");
        radioType = default("IdealRadio");
}