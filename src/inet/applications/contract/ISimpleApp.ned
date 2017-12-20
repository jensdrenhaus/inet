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

package inet.applications.contract;

//
// Template for simple applications, that are used in host without networklayer.
// Thus the packets are passt directly to the linklayer using 
// SimpleLinkLayerControllInfo attached to them as control info.
// It shows what gates an app needs.
//

moduleinterface ISimpleApp
{
	parameters:
	    @display("i=block/app");
	gates:
	    input appIn @labels(AppPayload/up);
	    output appOut @labels(AppPayload/down);
}