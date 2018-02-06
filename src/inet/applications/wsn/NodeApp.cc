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

#include "inet/applications/wsn/NodeApp.h"

#include "inet/applications/wsn/PageResponseMsg_m.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"

namespace inet {

using std::cout;

Define_Module(NodeApp);

enum SelfKinds {
    SEND = 1001,
};

NodeApp::NodeApp()
{
}

NodeApp::~NodeApp()
{
    cancelAndDelete(timer);
}

void NodeApp::initialize(int stage)
{
    WsnAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        processDelay = &par("processDelay");

        // state

        // statistics

        // references
        timer = new cMessage("sendPageResponse", SEND);

    }
}

void NodeApp::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
    getDisplayString().setTagArg("t", 0, buf);
}

void NodeApp::startOperating()
{
    ASSERT(!timer->isScheduled());
    lastStart = simTime();
    timer->setKind(SEND);
    sentCount = 0;
}

void NodeApp::stopOperating()
{
    lastStart = -1;
    cancelMsg();
}

void NodeApp::scheduleMsg(simtime_t now)
{
    simtime_t next;
    if (now < startTime) {
        EV_INFO << "App is not running yet";
        return;
    }
    else
        next = now + processDelay->doubleValue();
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void NodeApp::cancelMsg()
{
    cancelEvent(timer);
}

bool NodeApp::isEnabled()
{
    return true;
}

void NodeApp::processMessage(cPacket *msg)
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

void NodeApp::handleSelfMessage(cMessage *msg)
{
    ASSERT2(msg->getKind() == SEND, "Unknown kind in self message.");
    // send a message
    sendMsg();
}

void NodeApp::sendMsg()
{
    char name[32];
    sprintf(name, "response");

    PageResponseMsg *msg = new PageResponseMsg(name);
    ASSERT(nodeId != 0);
    msg->setOriginatorId(nodeId);
    //msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize);

    //emit(pingTxSeqSignal, sendSeqNo);
    //sendSeqNo++;
    sentCount++;
    SimpleLinkLayerControlInfo* controlInfo = new SimpleLinkLayerControlInfo;
    controlInfo->setSourceAddress(srcAddr);
    controlInfo->setDestinationAddress(MACAddress::BROADCAST_ADDRESS);

    msg->setControlInfo(dynamic_cast<cObject *>(controlInfo));
    EV_INFO << "Sending PageResponseMsg #" << msg->getSeqNo() << " to lower layer.\n";
    send(msg, "appOut");
}

void NodeApp::finish()
{
//    if (sendSeqNo == 0) {
//        if (printStat)
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
//    if (printStat) {
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

} //namespace
