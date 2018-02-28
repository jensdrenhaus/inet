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

#ifndef __INET_ACCESSPOINTAPP_H_
#define __INET_ACCESSPOINTAPP_H_

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/applications/wsn/WsnAppBase.h"

namespace inet {

using namespace std;
/**
 * TODO
 */
class INET_API AccessPointApp : public WsnAppBase
{
  protected:
    //parameters
    cPar* sendIntervalPar = nullptr;
    int count = 0;
    vector<unsigned int> productList;

    //state
    cMessage *timer = nullptr;    // to schedule the next message
    long sendSeqNo = 0;    // to match the response with the request that caused the response
    long expectedReplySeqNo = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;

    virtual void handleSelfMessage(cMessage* msg) override;
    virtual void processMessage(cPacket* msg) override;
    virtual void startOperating() override;
    virtual void stopOperating() override;
    virtual bool isEnabled() override;

    virtual void scheduleNextMsg(simtime_t previous);
    virtual void cancelNextMsg();
    virtual void sendMsg();

    virtual void refreshDisplay() const override;
    virtual void parseProductNumbers();
    virtual unsigned int toInt(const char* s);

  public:
    AccessPointApp();
    ~AccessPointApp();

};

} //namespace

#endif
