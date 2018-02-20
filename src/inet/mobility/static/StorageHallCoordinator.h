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

#ifndef __INET_STORAGEHALLCOORDINATOR_H_
#define __INET_STORAGEHALLCOORDINATOR_H_

#include <vector>
#include <set>

#include "inet/common/INETDefs.h"

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

using namespace std;
class Coord;

/**
 * TODO - Generated class
 */
class INET_API StorageHallCoordinator : public cSimpleModule
{
  protected:
    int numItems;

    double itemXdim;
    double itemYdim;
    double itemZdim;

    double side2sideDist;
    double back2backDist;
    double top2bottomDist;
    double interRowDist;

    double marginX;
    double marginY;
    double marginZ;

    int zLevels;
    int columns;
    int rows;

    int numSpots;
    int numFreeSpots;
    int numExtraSpots;
    vector<Coord> spots;
    set<int> occupied;



    double totalXdim;
    double totalYdim;
    double totalZdim;

  public:
    Coord getFreeSpot(bool includingExtraSpots = true);

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

  private:
    void checkDimensions();
    void calculateSpots();

  public:
    StorageHallCoordinator();
    ~StorageHallCoordinator();
};

} //namespace

#endif
