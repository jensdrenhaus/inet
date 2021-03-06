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

#ifndef __INET_APPLICATIONADAPTERBASE_H_
#define __INET_APPLICATIONADAPTERBASE_H_

#include "inet/common/INETDefs.h"


namespace inet {

/**
 * TODO - Generated class
 */
class ApplicationAdapterBase : public cSimpleModule
{
  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

  public:
    virtual void call_receptionNotify(unsigned long destId, unsigned long srcId, int msgId, int status) = 0;
    virtual void call_timerNotify(unsigned long nodeId) = 0;

    enum MsgKind {
        SendRequest = 991,
        ReceptionIndication = 992,
    };
};

} //namespace

#endif
