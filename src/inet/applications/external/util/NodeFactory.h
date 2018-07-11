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

#ifndef __INET_NODEFACTORY_H_
#define __INET_NODEFACTORY_H_

#include "inet/common/INETDefs.h"

#include "inet/applications/external/ExternalAppTrampoline.h"

namespace inet {

/**
 * TODO - Generated class
 */
class NodeFactory
{
  public:
    NodeFactory(const char* _nodeTypeName, cModule* _networkModule);
    ~NodeFactory();
    /*
     * creates a network node with the given physical address. Pass 0 to get a random address.
     * returns a pointer to the application module inside the new node
     */
    ExternalAppTrampoline* getNode(uint64 phy_address);

  private:
    /* fully qualified NED type name of the module which is to be dynamically instanciated */
    const char* nodeTypeName;
    cModule* networkModule;
    uint32 cnt;
};

} //namespace

#endif
