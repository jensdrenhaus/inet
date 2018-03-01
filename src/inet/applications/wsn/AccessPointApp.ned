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
        
        int numProducts = default(3);
        string productNumbers = default("*"); // product numbers to be requested, separated by spaces, "*" means every number from 1 to 'productCount'
        bool random = default(false); // whether to randomly change the product number or change it in a round-robin fashion
        
        volatile double sendInterval @unit("s") = default(3s); // time to wait between requests (can be random)
        int sendCount = default(2); // number of messages sent before change the product nr (> 0)
        
        volatile double sleepTime @unit("s") = default(10s); // sleep time before paging the next product (in addition to sendInterval)
        int changeCount = default(1); // number of product paging changes, -1 means forever (2 changes -> paging 3 products)
}
