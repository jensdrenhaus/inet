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

#include "inet/mobility/static/StorageHallCoordinator.h"

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
            checkDimensions();
            calculateSpots();

            totalXdim = 2*marginX+columns*(itemXdim+side2sideDist);
            totalYdim = 2*marginY+rows*(itemYdim+back2backDist);
            totalZdim = 2*marginZ+zLevels*(itemZdim+top2bottomDist);

        }

        else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {

        }

        else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
            ;
        }

        else if (stage == INITSTAGE_LAST) {
            cModule* module = this->getParentModule()->getParentModule();
            cDisplayString dispStr = module->getDisplayString();
            dispStr.setTagArg("bgb", 0, totalXdim);
            dispStr.setTagArg("bgb", 1, totalYdim);
        }
}

void StorageHallCoordinator::handleSelfMessage(cMessage *msg)
{
    ASSERT(false);
}

void StorageHallCoordinator::checkDimensions()
{
    printf("zLevels: %i \n"
           "columns: %i \n"
           "rows:    %i \n", zLevels,columns,rows);
    if(numItems > columns*rows*zLevels)
        cRuntimeError("ERROR: numItems(%i)> columns(%i)*rows(%i)*zLevel(%i) = %i \n", numItems, columns, rows, zLevels, numSpots);
    else
        printf("OK: %i items, %i places -> %i free + %i extra spots\n\n", numItems, numSpots, numFreeSpots, numExtraSpots);
}
void StorageHallCoordinator::calculateSpots()
{
    for(int index = 0; index < numSpots; index++) {
        int lev = index / (rows*columns);
        int row = (index/columns) % rows;
        int col = index % columns;

        double x = marginX+col*(itemXdim+side2sideDist)+((itemXdim+side2sideDist)/2);
        double y = marginY+row*(itemYdim+back2backDist)+((itemYdim+back2backDist)/2);
        double z = marginZ+lev*(itemZdim+top2bottomDist)+((itemZdim+top2bottomDist)/2);

        Coord spot = Coord(x,y,z);
        spots[index] = spot;
    }
    for(int i = 1; i <= numExtraSpots; i++) {
        double x = itemXdim/2+ side2sideDist/2;
        double y = itemYdim/2*i+back2backDist;
        double z = itemZdim/2;

        Coord spot = Coord(x,y,z);
        spots.push_back(spot);
    }
}

Coord StorageHallCoordinator::getFreeSpot(bool includingExtraSpots)
{
    pair<set<int>::iterator, bool> ret;
    int index = intuniform(0, numSpots-1);
    ret = occupied.insert(index);
    while(ret.second==false){
        index = intuniform(0, numSpots-1);
        ret = occupied.insert(index);
    }
    return spots[index];
}

} //namespace
