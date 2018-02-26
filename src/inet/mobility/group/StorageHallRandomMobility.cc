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

#include "inet/mobility/group/StorageHallRandomMobility.h"

#include "inet/mobility/group/StorageHallCoordinator.h"

namespace inet {

Define_Module(StorageHallRandomMobility);

void StorageHallRandomMobility::initialize(int stage)
{
    MassMobility::initialize(stage);
    EV_TRACE << "initializing StorageHallRandomMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        initAtCenter = par("initAtCenter").boolValue();
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        cModule* module = getModuleByPath("^.^.mobilityCoordinator");
        coordinator = check_and_cast<StorageHallCoordinator*>(module);
        constraintAreaMin = coordinator->getConstraintAreaMin();
        constraintAreaMax = coordinator->getConstraintAreaMax();
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        if(speedParameter->doubleValue() == 0)
            cancelEvent(moveTimer);
    }
}

void StorageHallRandomMobility::setInitialPosition()
{
    if (initAtCenter) {
        lastPosition.x = coordinator->getConstraintAreaMax().x / 2;
        lastPosition.y = coordinator->getConstraintAreaMax().y / 2;
        lastPosition.z = coordinator->getConstraintAreaMax().z / 2;
    }
    else
        lastPosition = getRandomPosition();
}


} //namespace
