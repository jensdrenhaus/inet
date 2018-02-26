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

#ifndef __INET_STATICSTORAGEHALLGRIDMOBILITY_H_
#define __INET_STATICSTORAGEHALLGRIDMOBILITY_H_

#include "inet/common/INETDefs.h"

#include "inet/mobility/base/LineSegmentsMobilityBase.h"
#include "inet/mobility/group/StorageHallMemberBase.h"

namespace inet {

class StorageHallCoordinator;
/**
 * TODO - Generated class
 */
class INET_API StorageHallGridMobility : public LineSegmentsMobilityBase,  public StorageHallMemberBase
{
  protected:
    StorageHallCoordinator* coordinator = nullptr;
    int mySpotIndex;
    bool nextMoveIsWait;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void setTargetPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void move() override;

  public:
    StorageHallGridMobility() {};
    virtual double getMaxSpeed() const override;
    void stopMoving() override;
};

} //namespace

#endif
