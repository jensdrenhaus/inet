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

#ifndef __INET_WSNAPPBASE_H_
#define __INET_WSNAPPBASE_H_

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"


namespace inet {

class IInterfaceTable;

/**
 * TODO - Generated class
 */
class INET_API WsnAppBase : public cSimpleModule, public ILifecycle
{
  protected:
    // parameters: for more details, see the corresponding NED parameters' documentation
    MACAddress srcAddr;
    int packetSize = 0;
    simtime_t startTime;
    simtime_t stopTime;
    bool printStat = false;


    // state
    unsigned long nodeId = 0;    // to determine which hosts are associated with the responses
    NodeStatus *nodeStatus = nullptr;    // lifecycle
    simtime_t lastStart;    // the last time when the app was started (lifecycle)
    IInterfaceTable* interfaceTable;

    // statistics
    cStdDev rttStat;
    static simsignal_t rttSignal;
    static simsignal_t numLostSignal;
    static simsignal_t numOutOfOrderArrivalsSignal;
    static simsignal_t pingTxSeqSignal;
    static simsignal_t pingRxSeqSignal;
    long sentCount = 0;    // number of sent Ping requests
    long lossCount = 0;    // number of lost requests
    long outOfOrderArrivalCount = 0;    // number of responses which arrived too late
    long numReplies = 0;    // number of received Ping requests

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleSelfMessage(cMessage* msg) = 0;
    virtual void processMessage(cPacket* msg) = 0;
    virtual void startOperating() = 0;
    virtual void stopOperating() = 0;
    virtual bool isEnabled() = 0;

    virtual bool isNodeUp();
    //virtual void processPingResponse(SimplePayload *msg);
    //virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt);

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    //virtual void refreshDisplay() const override;

  public:
    WsnAppBase();
    virtual ~WsnAppBase();
};

} //namespace

#endif
