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

#ifndef __INET_EXTERNALAPPTRAMPOLINE_H_
#define __INET_EXTERNALAPPTRAMPOLINE_H_

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/applications/external/ApplicationAdapterReceptionStates.h"


namespace inet {

class ExternalAppPayload;
class ApplicationAdapterBase;
class IInterfaceTable;

// how many ping request's send time is stored
#define PING_HISTORY_SIZE    100



/**
 * TODO - Generated class
 */
class ExternalAppTrampoline : public cSimpleModule, public ILifecycle, public cListener
{
  public:
    typedef enum nodeType_enum {
        UNDEFINED          = 0,
        ACCESSPOINT        = 1,
        PHYNODE_RESPONDING = 2,
        PHYNODE_IGNORING   = 3,
    } NodeType;

  protected:
    // parameters: for more details, see the corresponding NED parameters' documentation
    MACAddress destAddr;
    MACAddress srcAddr;
    std::vector<MACAddress> destAddresses;
    int packetSize = 0;
    int destAddrIdx = -1;
    bool printPing = false;
    ApplicationAdapterBase* adapter;
    IInterfaceTable* interfaceTable;
    NodeType nodeType = UNDEFINED;

    // state
    unsigned long nodeId;    // to determine which hosts are associated with the responses
    NodeStatus *nodeStatus = nullptr;    // lifecycle
    simtime_t lastStart;    // the last time when the app was started (lifecycle)
    long sendSeqNo = 0;    // to match the response with the request that caused the response
    long expectedReplySeqNo = 0;
    simtime_t sendTimeHistory[PING_HISTORY_SIZE];    // times of when the requests were sent
    cMessage* timer = nullptr;

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
    long numPongs = 0;    // number of received Ping requests
  public:
    static simsignal_t packetSentSignal;
    static simsignal_t packetReceivedOkSignal;
    static simsignal_t packetReceivedIgnoringSignal;
    static simsignal_t packetReceivedCorruptedSignal;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    //virtual void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details) override;
    virtual void finish() override;
    //virtual void refreshDisplay() const override;

    virtual void startSendingPingRequests();
    virtual void stopSendingPingRequests();
    virtual bool isNodeUp();
    virtual bool isEnabled();
    virtual void processMsg(ExternalAppPayload* msg);

    virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt);

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  public:
    unsigned long getNodeId();
    void setNodeType(NodeType type) {nodeType = type;}
    NodeType getNodeType() {return nodeType;}
    const char* getNodeTypeName();
    void sendMsg(unsigned long dest, int numBytes, int msgId);
    void wait(simtime_t duration);

    ExternalAppTrampoline();
    virtual ~ExternalAppTrampoline();
};

} //namespace

#endif
