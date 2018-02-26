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

#include "../group/StorageHallGridMobility.h"

#include <stdio.h>

#include "../group/StorageHallCoordinator.h"

namespace inet {


Define_Module(StorageHallGridMobility);

void StorageHallGridMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);
    EV_TRACE << "initializing StaticStorageHallGridMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        stationary = (par("speed").getType() == 'L' || par("speed").getType() == 'D') && (double)par("speed") == 0;
        mySpotIndex = -1;
        nextMoveIsWait = true;
        updateInterval = 0;
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        cModule* module = getModuleByPath("^.^.mobilityCoordinator");
        coordinator = check_and_cast<StorageHallCoordinator*>(module);
        constraintAreaMin = coordinator->getConstraintAreaMin();
        constraintAreaMax = coordinator->getConstraintAreaMax();
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        ;
    }
    else if (stage == INITSTAGE_LAST) {
        ;
    }
}

void StorageHallGridMobility::setInitialPosition()
{
    lastPosition = coordinator->getFreeSpot(&mySpotIndex);
    targetPosition = lastPosition;
}

void StorageHallGridMobility::setTargetPosition()
{
    if (nextMoveIsWait) {
        simtime_t waitTime = par("waitTime");
        nextChange = simTime() + waitTime;
        updateInterval = 0;
    }
    else {
        targetPosition = coordinator->getFreeSpot(&mySpotIndex); //getRandomPosition();
        double speed = par("speed");
        double distance = lastPosition.distance(targetPosition);
        simtime_t travelTime = distance / speed;
        nextChange = simTime() + travelTime;
        updateInterval = par("updateInterval");
    }
    nextMoveIsWait = !nextMoveIsWait;
}

void StorageHallGridMobility::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}

void StorageHallGridMobility::stopMoving()
{
    Enter_Method_Silent();
    cancelEvent(moveTimer);
}

double StorageHallGridMobility::getMaxSpeed() const
{
    return NaN;
}

} //namespace
