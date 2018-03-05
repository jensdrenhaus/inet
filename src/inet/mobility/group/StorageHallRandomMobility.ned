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

import inet.mobility.single.MassMobility;

//
// TODO auto-generated module
//
simple StorageHallRandomMobility extends MassMobility
{
    parameters:
        changeInterval = default(normal(4s, 0.1s));
        changeAngleBy = default(normal(0deg, 40deg));
        speed = default(0.5mps);
        bool initAtCenter = default(false);
        @class(StorageHallRandomMobility);
}
