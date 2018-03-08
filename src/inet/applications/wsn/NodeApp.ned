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
// TODO auto-generated module
//
simple NodeApp extends WsnAppBase
{
    parameters:
        @class(NodeApp);
        
        startTime @unit("s") = default(0s);
        stopTime @unit("s") = default(-1s); // -1 means no stop time
        packetSize @unit("B") = default(16B);
        
        int numProducts = default(3);
        int productNo = default(intuniform(1,numProducts));
        
        double fixProcessDelay @unit("ms") = default(0ms);
        volatile double randomProcessDelay @unit("us") = default(0us);
        volatile double processDelay = fixProcessDelay + randomProcessdelay;
}
