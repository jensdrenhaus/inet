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

package inet.mobility.group;

import inet.mobility.single.TractorMobility;

//
// TODO auto-generated module
//
simple StorageHallTractorMobility extends TractorMobility
{
    parameters:
        speed = default(0.5mps);
        x1 = 0m; // will be reset during network initialisation
        y1 = 0m; // will be reset during network initialisation
        x2 = 1m; // will be reset during network initialisation
        y2 = 1m; // will be reset during network initialisation
        rowCount = 1; // will be reset during network initialisation
        @class(StorageHallTractorMobility);
}
