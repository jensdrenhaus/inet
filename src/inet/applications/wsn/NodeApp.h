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

#ifndef __INET_NODEAPP_H_
#define __INET_NODEAPP_H_

#include "inet/common/INETDefs.h"

#include "inet/applications/wsn/WsnAppBase.h"


namespace inet {

/**
 * TODO - Generated class
 */
class INET_API NodeApp : public WsnAppBase
{
  protected:
    //parameters
    cPar* processDelay = nullptr;

    //state
    cMessage* timer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;

    virtual void handleSelfMessage(cMessage* msg) override;
    virtual void processMessage(cPacket* msg) override;
    virtual void startOperating() override;
    virtual void stopOperating() override;
    virtual bool isEnabled() override;

    virtual void scheduleMsg(simtime_t now);
    virtual void cancelMsg();
    virtual void sendMsg();

    virtual void refreshDisplay() const override;

  public:
    NodeApp();
    ~NodeApp();
};

} //namespace

#endif
