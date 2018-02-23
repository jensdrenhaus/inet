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

package inet.mobility.static;

import inet.mobility.base.MovingMobilityBase;

//
// TODO auto-generated module
//
simple StaticStorageHallGridMobility extends MovingMobilityBase
{
    parameters:
        volatile double speed @unit(mps) = default(0.5mps); // use uniform(minSpeed, maxSpeed) or another distribution
        volatile double waitTime @unit(s) = default(uniform(30s,90s)); // wait time between reaching a target and choosing a new one
        updateInterval @unit(s) = 1s; // the simulation time interval used to regularly signal mobility state changes and update the display
        @class(StaticStorageHallGridMobility);
}
