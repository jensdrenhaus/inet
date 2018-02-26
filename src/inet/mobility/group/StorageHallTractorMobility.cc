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

#include "inet/mobility/group/StorageHallTractorMobility.h"

#include "inet/mobility/group/StorageHallCoordinator.h"

namespace inet {

Define_Module(StorageHallTractorMobility);

StorageHallTractorMobility::StorageHallTractorMobility()
{
    coordinator = nullptr;
}

void StorageHallTractorMobility::initialize(int stage)
{
    TractorMobility::initialize(stage);

    EV_TRACE << "initializing StorageHallTractorMobility stage " << stage << endl;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        cModule* module = getModuleByPath("^.^.mobilityCoordinator");
        coordinator = check_and_cast<StorageHallCoordinator*>(module);
        constraintAreaMin = coordinator->getConstraintAreaMin();
        constraintAreaMax = coordinator->getConstraintAreaMax();

        double offsetX = coordinator->getMargin().x/2;
        double offsetY = coordinator->getMargin().y/2;
        x1 = coordinator->getConstraintAreaMin().x + offsetX;
        y1 = coordinator->getConstraintAreaMin().y + offsetY;
        x2 = coordinator->getConstraintAreaMax().x - offsetX;
        y2 = coordinator->getConstraintAreaMax().y - offsetY;
        double rows = ceil(double(coordinator->getNumRows())/2.0);
        rowCount = rows;
    }
}

void StorageHallTractorMobility::stopMoving()
{
    Enter_Method_Silent();
    cancelEvent(moveTimer);
}

} //namespace
