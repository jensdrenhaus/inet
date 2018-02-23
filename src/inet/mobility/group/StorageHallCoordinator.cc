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

#include "../group/StorageHallCoordinator.h"

#include "inet/common/geometry/common/Coord.h"

namespace inet {

Define_Module(StorageHallCoordinator);

StorageHallCoordinator::StorageHallCoordinator()
{

}

StorageHallCoordinator::~StorageHallCoordinator()
{

}

void StorageHallCoordinator::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV_TRACE << "initializing StorageHallCoordinator stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        numItems = par("numItems");

        itemXdim = par("itemXdim");
        itemYdim = par("itemYdim");
        itemZdim = par("itemZdim");

        side2sideDist = par("side2sideDist");
        back2backDist = par("back2backDist");
        top2bottomDist = par("top2bottomDist");
        interRowDist = par("interRowDist");

        marginX = par("marginX");
        marginY = par("marginY");
        marginZ = par("marginZ");

        zLevels = par("zLevels");
        columns = par("columns");
        rows = par("rows");

        numSpots = zLevels*columns*rows;
        numFreeSpots = numSpots-numItems;
        numExtraSpots = 2;
        spots = vector<Coord>(numSpots);
        spots.reserve(numSpots+numExtraSpots);
        free = list<int>();
        checkDimensions();
        calculateSpots();

        int corridors = (ceil((double(rows)/2)) -1);
        double interRowOffset = corridors*interRowDist;

        totalXdim = 2*marginX+columns*(itemXdim+side2sideDist);
        totalYdim = 2*marginY+rows*(itemYdim+back2backDist)+interRowOffset;
        totalZdim = 2*marginZ+zLevels*(itemZdim+top2bottomDist);
        printf("total x: %.1f m \n", totalXdim);
        printf("total y: %.1f m \n", totalYdim);
        printf("total z: %.1f m \n", totalZdim);


    }

    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        ;
    }

    else if (stage == INITSTAGE_LAST) {
        cModule* module = check_and_cast<cModule*>(this->getParentModule());
        cDisplayString& dispStr = module->getDisplayString();
        char str[32];
        sprintf(str, "bgu=m;bgb=%.1f,%.1f;bgg=1,2", totalXdim, totalYdim);
        dispStr.parse(str);
    }
}

void StorageHallCoordinator::handleMessage(cMessage *msg)
{
    ASSERT(false);
}

void StorageHallCoordinator::checkDimensions()
{
    printf("zLevels: %i \n"
           "columns: %i \n"
           "rows:    %i \n", zLevels,columns,rows);
    if(numItems > columns*rows*zLevels)
        throw cRuntimeError("ERROR: numItems(%li)> columns(%i)*rows(%i)*zLevel(%i) = %i \n", numItems, columns, rows, zLevels, numSpots);
    else
        printf("OK: %i items, %i places -> %i free + %i extra spots\n\n", numItems, numSpots, numFreeSpots, numExtraSpots);
}

void StorageHallCoordinator::calculateSpots()
{
    for (int i = 0; i < numSpots+numExtraSpots; i++)
                free.push_back(i);
    for(int index = 0; index < numSpots; index++) {
        int lev = index / (rows*columns);
        int row = (index/columns) % rows;
        int col = index % columns;
        int corridors = row/2;

        double x = marginX+col*(itemXdim+side2sideDist)+((itemXdim+side2sideDist)/2);
        double y = marginY+row*(itemYdim+back2backDist)+((itemYdim+back2backDist)/2)+corridors*interRowDist;
        double z = marginZ+lev*(itemZdim+top2bottomDist)+((itemZdim+top2bottomDist)/2);

        Coord spot = Coord(x,y,z);
        spots[index] = spot;
    }
    for(int i = 0; i < numExtraSpots; i++) {
        double x = itemXdim/2+side2sideDist/2;
        double y = itemYdim/2+back2backDist/2 + i*(itemYdim+back2backDist);
        double z = itemZdim/2;

        Coord spot = Coord(x,y,z);
        spots.push_back(spot);
    }
}

Coord StorageHallCoordinator::getFreeSpot(int* spotIndex)
{
    Enter_Method("get free spot");

    int x = free.size()-1;
    int i = intuniform(0, x);
    list<int>::iterator it = next(free.begin(), i); // it points to the ith element
    if (*spotIndex == -1) { // initial placement
        // check for extra spot
        while (*it >= numSpots) {
            i = intuniform(0, x);
            it = next(free.begin(), i);
        }
    }
    else { // add previous position to free
        free.push_back(*spotIndex);
    }
    int index = *it;
    free.erase(it);

    *spotIndex = index; // save new spotIndex
    return spots[index];
}

Coord StorageHallCoordinator::getConstraintAreaMax()
{
    Enter_Method_Silent();
    return Coord(totalXdim, totalYdim, totalZdim);
}

Coord StorageHallCoordinator::getConstraintAreaMin()
{
    Enter_Method_Silent();
    return Coord(0,0,0);
}

} //namespace
