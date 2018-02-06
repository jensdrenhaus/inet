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

package inet.applications.wsn;

import inet.applications.wsn.WsnAppBase;

//
// TODO 
//
simple AccessPointApp extends WsnAppBase
{
    parameters:
        @class(AccessPointApp);
        
        startTime @unit("s") = default(0s);
        stopTime @unit("s") = default(-1s);
        packetSize @unit("B") = default(16B);
        
        volatile double sendInterval @unit("s") = default(10s); // time to wait between pings (can be random)
        double count = default(-1); // number of messages sent, -1 means continuously
}
