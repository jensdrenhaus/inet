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

#ifndef __INET_LOGGER_H_
#define __INET_LOGGER_H_

#include "inet/common/INETDefs.h"

#include <iostream>
#include <fstream>
#include <unordered_map>


namespace inet {

using namespace std;
/**
 * TODO - Generated class
 */
class Logger : public cSimpleModule, public cListener
{
  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details) override;
    virtual void finish() override;

  private:
    ofstream resultFile;
    const char sep = ',';

    class PacketStat: public cObject
    {
      public:
        const char* nodeName = nullptr;
        int sent = 0;
        int receivedOk = 0;
        int receivedIgnoring = 0;
        int receivedCorrupted = 0;
    };

    std::unordered_map<int,PacketStat> nodeMap;
};

} //namespace

#endif
