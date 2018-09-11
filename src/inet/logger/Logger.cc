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

#include "inet/logger/Logger.h"

#include <iomanip>
#include <ctime>
#include <string>

#include "inet/applications/external/ExternalAppTrampoline.h"
#include "inet/physicallayer/energyconsumer/StateBasedMonitoringEpEnergyConsumer.h"

namespace inet {

using namespace physicallayer;

Define_Module(Logger);

void Logger::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // subscribe to signals
        getSimulation()->getSystemModule()->subscribe(ExternalAppTrampoline::packetSentSignal, this);
        getSimulation()->getSystemModule()->subscribe(ExternalAppTrampoline::packetReceivedOkSignal, this);
        getSimulation()->getSystemModule()->subscribe(ExternalAppTrampoline::packetReceivedIgnoringSignal, this);
        getSimulation()->getSystemModule()->subscribe(ExternalAppTrampoline::packetReceivedCorruptedSignal, this);
        getSimulation()->getSystemModule()->subscribe(StateBasedMonitoringEpEnergyConsumer::energyAccountChangedSignal, this);

        //open result file
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%y-%m-%d_%H:%M:%S");
        std::stringstream stream;
        stream << "./log/" << oss.str() <<".csv";
        std::string filename_string = stream.str();
        const char* filename = filename_string.c_str();

        //printf("%s \n", filename.c_str());
        resultFile.open(filename, ios::out | ios::app);
        if(resultFile.is_open()){
            resultFile << "Id,"
                    "Node,"
                    "Type,"
                    "NrSent,"
                    "NrReceivedOk,"
                    "NrReceivedIgnoring,"
                    "NrReceivedCorrupted,"
                    "totalEnergy,"
                    "ReceiverIdleEnergy,"
                    "ReceiverRecievingEnergy,"
                    "TransmitterIdealEnergy,"
                    "TransmitterTransmittingEnergy,"
                    "ReceiverBusyEnergy,"
                    "ReceiverReceivingPreambleEnergy,"
                    "ReceiverReceivingHeaderEnergy,"
                    "ReceiverReceivingDataEnergy,"
                    "TransmitterTransmittingPreambleEnergy,"
                    "TransmitterTransmittingHeaderEnergy,"
                    "TransmitterTransmittingDataEnergy,"
                    "OffEnergy,"
                    "SleepEnergy,"
                    "SwitchingEnergy" << endl;
        }
        else
            throw cRuntimeError("Logger: Unable to open the result file! Does the 'log' folder exist?");

        //init nodeMap
        //nodeMap = new std::unordered_map<int, cObject>;
        nodeMap.clear();
    }
}

void Logger::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
{
    if (id == ExternalAppTrampoline::packetReceivedOkSignal) {
        int nodeId = src->getParentModule()->getId();
        if (!nodeMap[nodeId].nodeName)
            nodeMap[nodeId].nodeName = src->getParentModule()->getName();
        if (!nodeMap[nodeId].type)
                    nodeMap[nodeId].type = check_and_cast<ExternalAppTrampoline*>(src)->getNodeTypeName();
        nodeMap[nodeId].receivedOk++;
    }
    else if (id == ExternalAppTrampoline::packetReceivedIgnoringSignal) {
        int nodeId = src->getParentModule()->getId();
        if (!nodeMap[nodeId].nodeName)
            nodeMap[nodeId].nodeName = src->getParentModule()->getName();
        if (!nodeMap[nodeId].type)
            nodeMap[nodeId].type = check_and_cast<ExternalAppTrampoline*>(src)->getNodeTypeName();
        nodeMap[nodeId].receivedIgnoring++;
    }
    else if (id == ExternalAppTrampoline::packetReceivedCorruptedSignal) {
        int nodeId = src->getParentModule()->getId();
        if (!nodeMap[nodeId].nodeName)
            nodeMap[nodeId].nodeName = src->getParentModule()->getName();
        if (!nodeMap[nodeId].type)
            nodeMap[nodeId].type = check_and_cast<ExternalAppTrampoline*>(src)->getNodeTypeName();
        nodeMap[nodeId].receivedCorrupted++;
    }
    else if (id == ExternalAppTrampoline::packetSentSignal) {
        int nodeId = src->getParentModule()->getId();
        if (!nodeMap[nodeId].nodeName)
            nodeMap[nodeId].nodeName = src->getParentModule()->getName();
        if (!nodeMap[nodeId].type)
            nodeMap[nodeId].type = check_and_cast<ExternalAppTrampoline*>(src)->getNodeTypeName();
        nodeMap[nodeId].sent++;
    }
}

void Logger::receiveSignal(cComponent* src, simsignal_t id, long value, cObject* details)
{
    if (id == StateBasedMonitoringEpEnergyConsumer::energyAccountChangedSignal) {
        double energy = check_and_cast<StateBasedMonitoringEpEnergyConsumer*>(src)->getEnergyFromAccount((StateBasedMonitoringEpEnergyConsumer::EnergyAccount)value);
        //printf("XXXXXXX account %ld energyVal %f\n",value,energy);
        int nodeId = src->getParentModule()->getParentModule()->getParentModule()->getId();
        nodeMap[nodeId].energyAccounts[value] += energy;
        nodeMap[nodeId].energyAccounts[StateBasedMonitoringEpEnergyConsumer::TOTAL_ACCOUNT] += energy;
    }
}

void Logger::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Logger: no messages expected!");
}

void Logger::finish()
{
    for (auto it : nodeMap){
        int id = it.first;
        PacketStat stat = it.second;
        resultFile << id << sep \
                << stat.nodeName << sep \
                << stat.type << sep \
                << stat.sent << sep \
                << stat.receivedOk << sep \
                << stat.receivedIgnoring << sep \
                << stat.receivedCorrupted << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::TOTAL_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::RECEIVER_IDLE_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::RECEIVER_RECEIVING_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::TRANSMITTER_IDLE_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::TRANSMITTER_TRANSMITTING_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::RECEIVER_BUSY_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::RECEIVER_RECEIVING_PREAMBLE_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::RECEIVER_RECEIVING_HEADER_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::RECEIVER_RECEIVING_DATA_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::TRANSMITTER_TRANSMITTING_PREAMBLE_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::TRANSMITTER_TRANSMITTING_HEADER_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::TRANSMITTER_TRANSMITTING_DATA_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::OFF_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::SLEEP_ACCOUNT] << sep \
                << stat.energyAccounts[StateBasedMonitoringEpEnergyConsumer::SWITCHING_ACCOUNT] << endl;
    }
}

} //namespace
