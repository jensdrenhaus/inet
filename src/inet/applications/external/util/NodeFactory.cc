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

#include "NodeFactory.h"

namespace inet {

NodeFactory::NodeFactory(const char* _nodeTypeName, cModule* _networkModule)
{
    cnt = 0;
    nodeTypeName = _nodeTypeName;
    networkModule = _networkModule;
}

NodeFactory::~NodeFactory()
{

}

DotnetCoreApp* NodeFactory::getNode(uint64 phy_address)
{
    cnt++;
    const char* spacer = (cnt < 10) ? "000" : (cnt < 100) ? "00" : (cnt < 1000) ? "0" : "";;
    char newNodeName[16];
    sprintf(newNodeName, "node%s%d", spacer, cnt);

    // find factory object
    cModuleType *moduleType = cModuleType::get(nodeTypeName);

    // create compound module and build its submodules
    cModule *newNode = moduleType->create(newNodeName, networkModule);
    newNode->finalizeParameters();
    newNode->buildInside();

    //manipulate parameter of submodules
    cModule* tempModule = newNode->getSubmodule("nic")->getSubmodule("mac");
    if (phy_address != 0){
        char addrStr[20];
        sprintf(addrStr, "%012lx", phy_address);
        tempModule->par("address") = addrStr;
    }
    else
        tempModule->par("address") = "auto"; // let MAC module create an address

    //initialize
    newNode->callInitialize();

    tempModule = newNode->getSubmodule("app");
    if(!tempModule)
        throw cRuntimeError("Cannot find Submodule 'app' in created Node");
    DotnetCoreApp* appPtr = check_and_cast<DotnetCoreApp*>(tempModule);
    return appPtr;
}

} //namespace
