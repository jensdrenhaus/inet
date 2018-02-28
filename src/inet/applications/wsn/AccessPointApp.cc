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

#include "inet/applications/wsn/AccessPointApp.h"

#include "inet/applications/wsn/PageMsg_m.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/mobility/group/StorageHallCoordinator.h"

namespace inet {

using std::cout;

Define_Module(AccessPointApp);

enum SelfKinds {
    PAGE_FIRST_PRODUCT = 1001,
    PAGE_CHANGE_PRODUCT,
    PAGE_SEND
};

AccessPointApp::AccessPointApp()
{
}

AccessPointApp::~AccessPointApp()
{
    cancelAndDelete(timer);
}

void AccessPointApp::initialize(int stage)
{
    WsnAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        sendIntervalPar = &par("sendInterval");
        sendCount = par("sendCount");
        sleepTimePar = &par("sleepTime");
        changeCount = par("changeCount");
        parseProductNumbers();
        random = par("random");
        productIndex = -1;
        productNr = 0;

        // state
        sendSeqNo = expectedReplySeqNo = 0;
        WATCH(sendSeqNo);
        WATCH(expectedReplySeqNo);

        // statistics

        // references
        timer = new cMessage("sendPage", PAGE_FIRST_PRODUCT);
    }
}

void AccessPointApp::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
    getDisplayString().setTagArg("t", 0, buf);
}

void AccessPointApp::startOperating()
{
    ASSERT(!timer->isScheduled());
    lastStart = simTime();
    timer->setKind(SEND);
    sentCount = 0;
    sendSeqNo = 0;
    scheduleNextMsg(-1);
}

void AccessPointApp::stopOperating()
{
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    cancelNextMsg();
}

void AccessPointApp::scheduleNextMsg(simtime_t previous)
{
    simtime_t next;
    if (previous < SIMTIME_ZERO)
        next = simTime() <= startTime ? startTime : simTime();
    else
        next = previous + sendIntervalPar->doubleValue();
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void AccessPointApp::cancelNextMsg()
{
    cancelEvent(timer);
}

bool AccessPointApp::isEnabled()
{
    return (count == -1 || sentCount < count);
}

void AccessPointApp::processMessage(cPacket *msg)
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

void AccessPointApp::handleSelfMessage(cMessage *msg)
{
    if (msg->getKind() == PAGE_FIRST_PRODUCT) {
        productIndex = random ? intuniform(0, (int)productList.size()-1) : 0;
        msg->setKind(PAGE_CHANGE_PRODUCT);
    }
    if (msg->getKind(PAGE_CHANGE_PRODUCT)) {
        if (productIndex >= (int)productList.size())
            throw cRuntimeError("Something is wrong here! productIndex is out of range");
        productNr = productList[productIndex];
        EV_INFO << "Paging product" << productNr << endl;
        msg->setKind(PAGE_SEND);
    }

    ASSERT2(msg->getKind() == PAGE_SEND, "Unknown kind in self message.");
    // send a message
    sendMsg();
    if (isEnabled())
        //
        scheduleNextMsg(simTime());
    else {
        cModule* module = getModuleByPath("^.^.mobilityCoordinator");
        StorageHallCoordinator* mobilityCoordinator = check_and_cast<StorageHallCoordinator*>(module);
        mobilityCoordinator->stopMoving();
    }
}

void AccessPointApp::sendMsg()
{
    char name[32];
    sprintf(name, "page%ld", sendSeqNo);

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
    EV_INFO << "Sending PageMsg #" << msg->getSeqNo() << " to lower layer.\n";
    send(msg, "appOut");
}

void AccessPointApp::parseProductNumbers()
{
    const char* productNrs = par("productNrs");
    if (!strcmp(productNrs, "*")) {
        if (par("productCount").longValue() < 1)
            throw cRuntimeError("productCount must be greater than 0!. Got %i", int(par("productCount")));
        unsigned int productCount = par("productCount");
        for (unsigned int i=1; i<= productCount; i++)
            productList.push_back(i);
    }
    else {
        if (!strcmp(productNrs, ""))
            throw cRuntimeError("You must specify at least one product number or use '*'! \n");
        cStringTokenizer tokenizer(productNrs);
        const char* token;
        while ((token = tokenizer.nextToken()) != nullptr) {
            int nr = toInt(token);
            productList.push_back(nr);
        }
    }
}

unsigned int AccessPointApp::toInt(const char* s)
{
    unsigned int num = 0;
    while (true) {
        if (*s < '0' || *s > '9')
            throw cRuntimeError("Cannot parse product number. Illegal character: '%c' ! \n", *s);
        while (*s >= '0' && *s <= '9') {
            num = 10*num + (*s - '0');
            if (num > 65535)
                throw cRuntimeError("Cannot parse product number. Number too big: '%i' ! \n", num);
            s++;
        }
        if (num < 1)
            throw cRuntimeError("Cannot parse product number. Must be greater the 0. Got: '%i' ! \n", num);
        if (!*s)
            break; //end of string
    }
    return num;
}

void AccessPointApp::finish()
{
    if (sendSeqNo == 0) {
        if (printStat)
            EV_DETAIL << getFullPath() << ": No pings sent, skipping recording statistics and printing results.\n";
        return;
    }

    lossCount += sendSeqNo - expectedReplySeqNo;
    // record statistics
    recordScalar("Pings sent", sendSeqNo);
    recordScalar("ping loss rate (%)", 100 * lossCount / (double)sendSeqNo);
    recordScalar("ping out-of-order rate (%)", 100 * outOfOrderArrivalCount / (double)sendSeqNo);

    // print it to stdout as well
    if (printStat) {
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
