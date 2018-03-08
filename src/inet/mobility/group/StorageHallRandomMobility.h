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

#ifndef __INET_STORAGEHALLRANDOMMOBILITY_H_
#define __INET_STORAGEHALLRANDOMMOBILITY_H_

#include "inet/common/INETDefs.h"

#include "inet/mobility/single/MassMobility.h"
#include "inet/mobility/group/StorageHallMemberBase.h"

namespace inet {

class StorageHallCoordinator;

/**
 * TODO - Generated class
 */
class INET_API StorageHallRandomMobility : public MassMobility, public StorageHallMemberBase
{
  protected:
    bool initAtCenter;
    StorageHallCoordinator* coordinator;

  public:
    void stopMoving() override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage) override;

    /** @brief Initializes the position from the display string or from module parameters. */
    virtual void setInitialPosition() override;

  public:
    StorageHallRandomMobility() {}
};

} //namespace

#endif
