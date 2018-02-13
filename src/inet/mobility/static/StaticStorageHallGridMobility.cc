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

#include <stdio.h>

#include "inet/mobility/static/StaticStorageHallGridMobility.h"

namespace inet {


Define_Module(StaticStorageHallGridMobility);

void StaticStorageHallGridMobility::initialize(int stage)
{
    StationaryMobility::initialize(stage);
    EV_TRACE << "initializing StaticStorageHallGridMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        ;
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {

    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        ;
    }
    else if (stage == INITSTAGE_LAST) {
        ;
    }
}

void StaticStorageHallGridMobility::setInitialPosition()
{
    //int index = visualRepresentation->getIndex();


}

} //namespace
