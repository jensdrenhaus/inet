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

#include "../wsn/SimpleApp.h"

#include <iostream>
#include <string>

#include "../wsn/PageMsg_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"

namespace inet {

using std::cout;

Define_Module(SimpleApp);

simsignal_t SimpleApp::rttSignal = registerSignal("rtt");
simsignal_t SimpleApp::numLostSignal = registerSignal("numLost");
simsignal_t SimpleApp::numOutOfOrderArrivalsSignal = registerSignal("numOutOfOrderArrivals");
simsignal_t SimpleApp::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t SimpleApp::pingRxSeqSignal = registerSignal("pingRxSeq");

enum SelfKinds {
    SEND = 1001,
};

SimpleApp::SimpleApp()
{
}

SimpleApp::~SimpleApp()
{
    cancelAndDelete(timer);
}

void SimpleApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        packetSize = par("packetSize");
        sendIntervalPar = &par("sendInterval");
        count = par("count");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        printStat = par("printPing").boolValue();

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

        // references
        timer = new cMessage("sendPing", SEND);
        cModule* module = this->getParentModule()->getSubmodule("interfaceTable");
        interfaceTable = check_and_cast<IInterfaceTable*>(module);


    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // read MAC address from NIC and generate NodeId
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
        if (isEnabled() && isNodeUp())
            startOperating();
    }
}

void SimpleApp::handleMessage(cMessage *msg)
{
    if (!isNodeUp()) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Self message '%s' received when %s is down", msg->getName(), getComponentType()->getName());
        else {
            EV_WARN << "PingApp is down, dropping '" << msg->getName() << "' message\n";
            delete msg;
            return;
        }
    }
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else {
        cPacket* payload = check_and_cast<cPacket*>(msg);
        processMessage(payload);
    }
}

void SimpleApp::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
    getDisplayString().setTagArg("t", 0, buf);
}

bool SimpleApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isEnabled())
            startOperating();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            stopOperating();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stopOperating();
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void SimpleApp::startOperating()
{
    ASSERT(!timer->isScheduled());
    lastStart = simTime();
    timer->setKind(SEND);
    sentCount = 0;
    sendSeqNo = 0;
    scheduleNextMsg(-1);
}

void SimpleApp::stopOperating()
{
    nodeId = 0;
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    cancelNextMsg();
}

void SimpleApp::scheduleNextMsg(simtime_t previous)
{
    simtime_t next;
    if (previous < SIMTIME_ZERO)
        next = simTime() <= startTime ? startTime : simTime();
    else
        next = previous + sendIntervalPar->doubleValue();
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void SimpleApp::cancelNextMsg()
{
    cancelEvent(timer);
}

bool SimpleApp::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool SimpleApp::isEnabled()
{
    return (count == -1 || sentCount < count);
}

void SimpleApp::processMessage(cPacket *msg)
{
//    // change addresses and send out reply
//    SimpleLinkLayerControlInfo *ctrl = check_and_cast<SimpleLinkLayerControlInfo *>(msg->getControlInfo());
//    //MACAddress src = ctrl->getSourceAddress();
//    //MACAddress dest = ctrl->getDestinationAddress();
//    ctrl->setDestinationAddress(MACAddress::BROADCAST_ADDRESS);
//    ctrl->setSourceAddress(MACAddress::UNSPECIFIED_ADDRESS);
//    msg->setName((std::string(msg->getName()) + "-reply").c_str());
//    msg->setIsReply(true);
//
//    send(msg, "appOut");
}

void SimpleApp::handleSelfMessage(cMessage *msg)
{
    ASSERT2(msg->getKind() == SEND, "Unknown kind in self message.");
    // send a message
    sendMsg();
    if (isEnabled())
        scheduleNextMsg(simTime());
}

//void SimpleApp::processPingResponse(SimplePayload *msg)
//{
//    if (msg->getOriginatorId() != pid) {
//        EV_WARN << "Received response was not sent by this application, dropping packet\n";
//        delete msg;
//        return;
//    }
//
//    // get src, hopCount etc from packet, and print them
//    SimpleLinkLayerControlInfo *ctrl = check_and_cast<SimpleLinkLayerControlInfo *>(msg->getControlInfo());
//    MACAddress src = ctrl->getSourceAddress();
//    //L3Address dest = ctrl->getDestinationAddress();
//
//    // calculate the RTT time by looking up the the send time of the packet
//    // if the send time is no longer available (i.e. the packet is very old and the
//    // sendTime was overwritten in the circular buffer) then we just return a 0
//    // to signal that this value should not be used during the RTT statistics)
//    simtime_t rtt = sendSeqNo - msg->getSeqNo() > PING_HISTORY_SIZE ?
//        0 : simTime() - sendTimeHistory[msg->getSeqNo() % PING_HISTORY_SIZE];
//
//    if (printPing) {
//        cout << getFullPath() << ": reply of " << std::dec << msg->getByteLength()
//             << " bytes from " << src
//             << " icmp_seq=" << msg->getSeqNo()
//             << " time=" << (rtt * 1000) << " msec"
//             << " (" << msg->getName() << ")" << endl;
//    }
//
//    // update statistics
//    countPingResponse(msg->getByteLength(), msg->getSeqNo(), rtt);
//    delete msg;
//}

//void SimpleApp::countPingResponse(int bytes, long seqNo, simtime_t rtt)
//{
//    EV_INFO << "Ping reply #" << seqNo << " arrived, rtt=" << rtt << "\n";
//    emit(pingRxSeqSignal, seqNo);
//
//    numPongs++;
//
//    // count only non 0 RTT values as 0s are invalid
//    if (rtt > 0) {
//        rttStat.collect(rtt);
//        emit(rttSignal, rtt);
//    }
//
//    if (seqNo == expectedReplySeqNo) {
//        // expected ping reply arrived; expect next sequence number
//        expectedReplySeqNo++;
//    }
//    else if (seqNo > expectedReplySeqNo) {
//        EV_DETAIL << "Jump in seq numbers, assuming pings since #" << expectedReplySeqNo << " got lost\n";
//
//        // jump in the sequence: count pings in gap as lost for now
//        // (if they arrive later, we'll decrement back the loss counter)
//        long jump = seqNo - expectedReplySeqNo;
//        lossCount += jump;
//        emit(numLostSignal, lossCount);
//
//        // expect sequence numbers to continue from here
//        expectedReplySeqNo = seqNo + 1;
//    }
//    else {    // seqNo < expectedReplySeqNo
//              // ping reply arrived too late: count as out-of-order arrival (not loss after all)
//        EV_DETAIL << "Arrived out of order (too late)\n";
//        outOfOrderArrivalCount++;
//        lossCount--;
//        emit(numOutOfOrderArrivalsSignal, outOfOrderArrivalCount);
//        emit(numLostSignal, lossCount);
//    }
//}

void SimpleApp::sendMsg()
{
    char name[32];
    sprintf(name, "msg%ld", sendSeqNo);

    PageMsg *msg = new PageMsg(name);
    ASSERT(nodeId != 0);
    msg->setOriginatorId(nodeId);
    msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize);

    emit(pingTxSeqSignal, sendSeqNo);
    sendSeqNo++;
    sentCount++;
    SimpleLinkLayerControlInfo* controlInfo = new SimpleLinkLayerControlInfo;
    controlInfo->setSourceAddress(srcAddr);
    controlInfo->setDestinationAddress(MACAddress::BROADCAST_ADDRESS);

    msg->setControlInfo(dynamic_cast<cObject *>(controlInfo));
    EV_INFO << "Sending ping request #" << msg->getSeqNo() << " to lower layer.\n";
    send(msg, "appOut");
}

void SimpleApp::finish()
{
    if (sendSeqNo == 0) {
        if (printPing)
            EV_DETAIL << getFullPath() << ": No pings sent, skipping recording statistics and printing results.\n";
        return;
    }

    lossCount += sendSeqNo - expectedReplySeqNo;
    // record statistics
    recordScalar("Pings sent", sendSeqNo);
    recordScalar("ping loss rate (%)", 100 * lossCount / (double)sendSeqNo);
    recordScalar("ping out-of-order rate (%)", 100 * outOfOrderArrivalCount / (double)sendSeqNo);

    // print it to stdout as well
    if (printPing) {
        cout << "--------------------------------------------------------" << endl;
        cout << "\t" << getFullPath() << endl;
        cout << "--------------------------------------------------------" << endl;

        cout << "sent: " << sendSeqNo << "   received: " << numPongs << "   loss rate (%): " << (100 * lossCount / (double)sendSeqNo) << endl;
        cout << "round-trip min/avg/max (ms): " << (rttStat.getMin() * 1000.0) << "/"
             << (rttStat.getMean() * 1000.0) << "/" << (rttStat.getMax() * 1000.0) << endl;
        cout << "stddev (ms): " << (rttStat.getStddev() * 1000.0) << "   variance:" << rttStat.getVariance() << endl;
        cout << "--------------------------------------------------------" << endl;
    }
}

} //namespace
