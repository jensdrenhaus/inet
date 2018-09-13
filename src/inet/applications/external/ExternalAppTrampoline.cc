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

#include <iostream>
#include <string>

#include "inet/applications/external/ExternalAppTrampoline.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/applications/external/ExternalAppPayload_m.h"
#include "inet/applications/external/AdapterMsg_m.h"
#include "inet/applications/external/ApplicationAdapterBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"


namespace inet {

using std::cout;

Define_Module(ExternalAppTrampoline);

simsignal_t ExternalAppTrampoline::rttSignal = registerSignal("rtt");
simsignal_t ExternalAppTrampoline::numLostSignal = registerSignal("numLost");
simsignal_t ExternalAppTrampoline::numOutOfOrderArrivalsSignal = registerSignal("numOutOfOrderArrivals");
simsignal_t ExternalAppTrampoline::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t ExternalAppTrampoline::pingRxSeqSignal = registerSignal("pingRxSeq");

simsignal_t ExternalAppTrampoline::packetSentSignal = registerSignal("packetSent");
simsignal_t ExternalAppTrampoline::packetReceivedOkSignal = registerSignal("packetReceivedOk");
simsignal_t ExternalAppTrampoline::packetReceivedIgnoringSignal = registerSignal("packetReceivedIgnoring");
simsignal_t ExternalAppTrampoline::packetReceivedCorruptedSignal = registerSignal("packetReceivedCorrupted");

#define TIMER_KIND 999


ExternalAppTrampoline::ExternalAppTrampoline()
{
    timer = nullptr;
}

ExternalAppTrampoline::~ExternalAppTrampoline()
{
    cancelAndDelete(timer);
}

void ExternalAppTrampoline::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        // addresses will be assigned later by ApplicationAdapter)
        packetSize = 0;
        printPing = par("printPing").boolValue();

        // state
        lastStart = -1;
        sendSeqNo = expectedReplySeqNo = 0;
        WATCH(sendSeqNo);
        WATCH(expectedReplySeqNo);

        // statistics
        rttStat.setName("pingRTT");
        sentCount = lossCount = outOfOrderArrivalCount = numPongs = 0;
        WATCH(lossCount);
        WATCH(outOfOrderArrivalCount);
        WATCH(numPongs);

        timer = new cMessage("timer");
        timer->setKind(TIMER_KIND);

        cModule* module = getParentModule()->getParentModule()->getSubmodule("adapter");
        adapter = check_and_cast<ApplicationAdapterBase*>(module);
        module = this->getParentModule()->getSubmodule("interfaceTable");
        interfaceTable = check_and_cast<IInterfaceTable*>(module);
        module = this->getParentModule()->getSubmodule("nic")->getSubmodule("radio");
        module->subscribe("receptionEndedIgnoring", this);
        module = this->getParentModule()->getSubmodule("nic")->getSubmodule("mac");
        module->subscribe("packetFromLowerDropped", this);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // generate nodeId from MAC Adress
        InterfaceEntry* e = interfaceTable->getInterfaceByName("nic");
        srcAddr = e->getMacAddress();
        if (srcAddr.isUnspecified() || srcAddr.isBroadcast())
            throw cRuntimeError("Invalid source address!");
        nodeId = (unsigned long)srcAddr.getInt();

        // visualize MAC Address
        cDisplayString& dispStr = this->getParentModule()->getDisplayString();
        dispStr.setTagArg("tt", 0, srcAddr.str().c_str());
        dispStr.setTagArg("t", 0, srcAddr.str().c_str());
        // startup
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
//        if (isEnabled() && isNodeUp())
//            startSendingPingRequests();
    }
}

void ExternalAppTrampoline::handleMessage(cMessage *msg)
{
    if (!isNodeUp()) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Self message '%s' received when %s is down", msg->getName(), getComponentType()->getName());
        else {
            EV_WARN << "App is down, dropping '" << msg->getName() << "' message\n";
            delete msg;
            return;
        }
    }
    if (msg->isSelfMessage()) {
        if (msg->getKind() == TIMER_KIND)
            adapter->call_timerNotify(nodeId);
        else
            throw cRuntimeError("Unexpeced self message");
    }
    else if (msg->arrivedOn("adapterIn")){
        AdapterMsg* sendReq = check_and_cast<AdapterMsg*>(msg);
        if(msg->getKind() == ApplicationAdapterBase::SendRequest){
            unsigned long destId = sendReq->getDestId();
            int numBytes = sendReq->getNumBytes();
            int msgId = sendReq->getMsgId();
            delete(sendReq);
            sendMsg(destId, numBytes, msgId);
        }
    }
    else if (msg->arrivedOn("bypass")) {
        ExternalAppPayload* ignoredMsg = check_and_cast<ExternalAppPayload*>(msg);
        if(ignoredMsg->getDestinationId() == nodeId || ignoredMsg->getDestinationId() == 0xFFFFFFFFFFFF){
            emit(packetReceivedIgnoringSignal, msg);
            AdapterMsg* ind = new AdapterMsg("indication");
            ind->setKind(ApplicationAdapterBase::ReceptionIndication);
            ind->setDestId(nodeId);
            ind->setSrcId(ignoredMsg->getOriginatorId());
            ind->setMsgId(ignoredMsg->getExtMsgId());
            ind->setStatus((int)IGNORED);
            delete(ignoredMsg);
            sendDirect(ind, adapter->gate("appsIn"));
        }
    }
    else if (msg->arrivedOn("appIn")){
        ExternalAppPayload* receivedMsg = check_and_cast<ExternalAppPayload*>(msg);
        processMsg(receivedMsg);
    }
    else {
        throw cRuntimeError("unknown Message in ExternalAppTrampoline");
    }
}

void ExternalAppTrampoline::processMsg(ExternalAppPayload* msg)
{
    emit(packetReceivedOkSignal, msg);
    //adapter->call_receptionNotify(nodeId, msg->getOriginatorId(), msg->getExtMsgId(), OK);

    AdapterMsg* ind = new AdapterMsg("indication");
    ind->setKind(ApplicationAdapterBase::ReceptionIndication);
    ind->setDestId(nodeId);
    ind->setSrcId(msg->getOriginatorId());
    ind->setMsgId(msg->getExtMsgId());
    ind->setStatus((int)OK);
    delete msg;
    sendDirect(ind, adapter->gate("appsIn"));
    printf("######### sent App --> Adapter ###########\n");
}

//void ExternalAppTrampoline::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
//{
//    if(id == physicallayer::Radio::receptionEndedIgnoringSignal || id == LayeredProtocolBase::packetFromLowerDroppedSignal){
//        cPacket* pkg = dynamic_cast<cPacket*>(value);
//        if(!pkg) return;
//        ExternalAppPayload* msg = dynamic_cast<ExternalAppPayload*>(pkg->getEncapsulatedPacket());
//        if(!msg) return;
//        if(msg->getDestinationId() == nodeId || msg->getDestinationId() == 0xFFFFFFFFFFFF){
////            if(id == physicallayer::Radio::receptionEndedIgnoringSignal){
////                emit(packetReceivedIgnoringSignal, msg);
////                //adapter->call_receptionNotify(nodeId, msg->getOriginatorId(), msg->getExtMsgId(), IGNORED);
////
////                AdapterMsg* ind = new AdapterMsg("indication");
////                ind->setKind(ApplicationAdapterBase::ReceptionIndication);
////                ind->setDestId(nodeId);
////                ind->setSrcId(msg->getOriginatorId());
////                ind->setMsgId(msg->getExtMsgId());
////                ind->setStatus((int)IGNORED);
////                sendDirect(ind, adapter->gate("appsIn"));
////            }
//            if(id == LayeredProtocolBase::packetFromLowerDroppedSignal){
//                emit(packetReceivedCorruptedSignal, msg);
//                //adapter->call_receptionNotify(nodeId, msg->getOriginatorId(), msg->getExtMsgId(), BITERROR);
//
//                AdapterMsg* ind = new AdapterMsg("indication");
//                ind->setKind(ApplicationAdapterBase::ReceptionIndication);
//                ind->setDestId(nodeId);
//                ind->setSrcId(msg->getOriginatorId());
//                ind->setMsgId(msg->getExtMsgId());
//                ind->setStatus((int)BITERROR);
//                sendDirect(ind, adapter->gate("appsIn"));
//            }
//        }
//    }
//}

//void ExternalApp::refreshDisplay() const
//{
//    char buf[40];
//    sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
//    getDisplayString().setTagArg("t", 0, buf);
//}

bool ExternalAppTrampoline::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isEnabled())
            //startSendingPingRequests();
            throw cRuntimeError("implement handleOperationSatge in ExternalApp");
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            //stopSendingPingRequests();
            throw cRuntimeError("implement handleOperationSatge in ExternalApp");
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            //stopSendingPingRequests();
            throw cRuntimeError("implement handleOperationSatge in ExternalApp");
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void ExternalAppTrampoline::startSendingPingRequests()
{
    ASSERT(!timer->isScheduled());
    lastStart = simTime();
    sentCount = 0;
    sendSeqNo = 0;
}

void ExternalAppTrampoline::stopSendingPingRequests()
{
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    srcAddr = destAddr = MACAddress();
    destAddresses.clear();
    destAddrIdx = -1;
}

bool ExternalAppTrampoline::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool ExternalAppTrampoline::isEnabled()
{
    //return par("destAddr").stringValue()[0] && (count == -1 || sentCount < count);
    return true;
}

void ExternalAppTrampoline::countPingResponse(int bytes, long seqNo, simtime_t rtt)
{
    EV_INFO << "Ping reply #" << seqNo << " arrived, rtt=" << rtt << "\n";
    emit(pingRxSeqSignal, seqNo);

    numPongs++;

    // count only non 0 RTT values as 0s are invalid
    if (rtt > 0) {
        rttStat.collect(rtt);
        emit(rttSignal, rtt);
    }

    if (seqNo == expectedReplySeqNo) {
        // expected ping reply arrived; expect next sequence number
        expectedReplySeqNo++;
    }
    else if (seqNo > expectedReplySeqNo) {
        EV_DETAIL << "Jump in seq numbers, assuming pings since #" << expectedReplySeqNo << " got lost\n";

        // jump in the sequence: count pings in gap as lost for now
        // (if they arrive later, we'll decrement back the loss counter)
        long jump = seqNo - expectedReplySeqNo;
        lossCount += jump;
        emit(numLostSignal, lossCount);

        // expect sequence numbers to continue from here
        expectedReplySeqNo = seqNo + 1;
    }
    else {    // seqNo < expectedReplySeqNo
              // ping reply arrived too late: count as out-of-order arrival (not loss after all)
        EV_DETAIL << "Arrived out of order (too late)\n";
        outOfOrderArrivalCount++;
        lossCount--;
        emit(numOutOfOrderArrivalsSignal, outOfOrderArrivalCount);
        emit(numLostSignal, lossCount);
    }
}

void ExternalAppTrampoline::finish()
{
//    if (sendSeqNo == 0) {
//        if (printPing)
//            EV_DETAIL << getFullPath() << ": No pings sent, skipping recording statistics and printing results.\n";
//        return;
//    }
//
//    lossCount += sendSeqNo - expectedReplySeqNo;
//    // record statistics
//    recordScalar("Pings sent", sendSeqNo);
//    recordScalar("ping loss rate (%)", 100 * lossCount / (double)sendSeqNo);
//    recordScalar("ping out-of-order rate (%)", 100 * outOfOrderArrivalCount / (double)sendSeqNo);
//
//    // print it to stdout as well
//    if (printPing) {
//        cout << "--------------------------------------------------------" << endl;
//        cout << "\t" << getFullPath() << endl;
//        cout << "--------------------------------------------------------" << endl;
//
//        cout << "sent: " << sendSeqNo << "   received: " << numPongs << "   loss rate (%): " << (100 * lossCount / (double)sendSeqNo) << endl;
//        cout << "round-trip min/avg/max (ms): " << (rttStat.getMin() * 1000.0) << "/"
//             << (rttStat.getMean() * 1000.0) << "/" << (rttStat.getMax() * 1000.0) << endl;
//        cout << "stddev (ms): " << (rttStat.getStddev() * 1000.0) << "   variance:" << rttStat.getVariance() << endl;
//        cout << "--------------------------------------------------------" << endl;
//    }
}

unsigned long ExternalAppTrampoline::getNodeId()
{
    return nodeId;
}


void ExternalAppTrampoline::sendMsg(unsigned long dest, int numBytes, int msgId)
{
    //Enter_Method_Silent("send");

    char name[32];
    sprintf(name, "msg%ld", sendSeqNo);

    ExternalAppPayload *msg = new ExternalAppPayload(name);
    //ASSERT(pid != -1);
    msg->setOriginatorId(nodeId);
    msg->setDestinationId(dest);
    msg->setByteLength(numBytes);
    msg->setExtMsgId(msgId);

    sendSeqNo++;
    sentCount++;

    SimpleLinkLayerControlInfo* controlInfo = new SimpleLinkLayerControlInfo;
    controlInfo->setSourceAddress(srcAddr);
    controlInfo->setDestinationAddress(MACAddress(dest));

    msg->setControlInfo(dynamic_cast<cObject *>(controlInfo));
    EV_INFO << "Sending message #" << sendSeqNo-1 << " to lower layer.\n";
    emit(packetSentSignal, msg);
    send(msg, "appOut");
}

void ExternalAppTrampoline::wait(simtime_t duration)
{
    //Enter_Method("wait");

    scheduleAt(simTime()+duration, timer);
}

const char* ExternalAppTrampoline::getNodeTypeName()
{
    //Enter_Method_Silent();

    switch(nodeType){
    case UNDEFINED:
        return "undefined";
        break;
    case PHYNODE_RESPONDING:
        return "PhyNode_responding";
        break;
    case PHYNODE_IGNORING:
        return "PhyNode_ignoring";
        break;
    case ACCESSPOINT:
        return "AccessPoint";
        break;
    default:
        throw cRuntimeError("unknown NodeType");
    }
}

} //namespace
