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

#include "inet/applications/wsn/WsnAppBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Register_Abstract_Class(WsnAppBase);

simsignal_t WsnAppBase::rttSignal = registerSignal("rtt");
simsignal_t WsnAppBase::numLostSignal = registerSignal("numLost");
simsignal_t WsnAppBase::numOutOfOrderArrivalsSignal = registerSignal("numOutOfOrderArrivals");
simsignal_t WsnAppBase::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t WsnAppBase::pingRxSeqSignal = registerSignal("pingRxSeq");

WsnAppBase::WsnAppBase()
{
}

WsnAppBase::~WsnAppBase()
{
}

void WsnAppBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        packetSize = par("packetSize");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        printStat = par("printStat").boolValue();

        // state
        lastStart = -1;

        // statistics
        rttStat.setName("pingRTT");
        sentCount = lossCount = outOfOrderArrivalCount = numReplies = 0;
        WATCH(lossCount);
        WATCH(outOfOrderArrivalCount);
        WATCH(numReplies);

        // references
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
        dispStr.setTagArg("tt", 0, srcAddr.str().c_str()); // tooltip
        //dispStr.setTagArg("t", 0, srcAddr.str().c_str()); // text

        // startup
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        if (isEnabled() && isNodeUp())
            startOperating();
    }
}

void WsnAppBase::handleMessage(cMessage *msg)
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

//void WsnAppBase::refreshDisplay() const
//{
//    char buf[40];
//    sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numReplies);
//    getDisplayString().setTagArg("t", 0, buf);
//}

bool WsnAppBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
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

bool WsnAppBase::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}


} //namespace
