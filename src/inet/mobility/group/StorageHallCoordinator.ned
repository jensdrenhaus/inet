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

import inet.mobility.base.MobilityBase;

//
// TODO auto-generated module
//
simple StorageHallCoordinator extends MobilityBase
{
    parameters:
        int numItems;
		
		double itemXdim @unit("m") = default(0.3m);
		double itemYdim @unit("m") = default(0.4m);
		double itemZdim @unit("m") = default(0.2m);
		
		double side2sideDist @unit("m") = default(0.2m);
		double back2backDist @unit("m") = default(0.1m);
		double top2bottomDist @unit("m") = default(0.3m);
		double interRowDist @unit("m") = default(1.2m);
		
		double marginX @unit("m") = default(1m);
		double marginY @unit("m") = default(1m);
		double marginZ @unit("m") = default(1m);
		
		int zLevels = default(1);
		int columns = default(ceil(sqrt(numItems/zLevels*2)));
		int rows = default((ceil(numItems/zLevels) + columns -1) / columns);
    
    	@class(StorageHallCoordinator);
}
