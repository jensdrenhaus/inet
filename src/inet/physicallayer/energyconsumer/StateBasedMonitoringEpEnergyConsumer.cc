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

#include "inet/physicallayer/energyconsumer/StateBasedMonitoringEpEnergyConsumer.h"

namespace inet {

namespace physicallayer {

simsignal_t StateBasedMonitoringEpEnergyConsumer::energyAccountChangedSignal = cComponent::registerSignal("EnergyAccountChanged");


Define_Module(StateBasedMonitoringEpEnergyConsumer);

void StateBasedMonitoringEpEnergyConsumer::initialize(int stage)
{
    StateBasedEpEnergyConsumer::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        totalEnergyAccount = J(0);
        offEnergyAccount = J(0);
        sleepEnergyAccount = J(0);
        switchingEnergyAccount = J(0);
        receiverIdleEnergyAccount = J(0);
        receiverBusyEnergyAccount = J(0);
        receiverReceivingEnergyAccount = J(0);
        receiverReceivingPreambleEnergyAccount = J(0);
        receiverReceivingHeaderEnergyAccount = J(0);
        receiverReceivingDataEnergyAccount = J(0);
        transmitterIdleEnergyAccount = J(0);
        transmitterTransmittingEnergyAccount = J(0);
        transmitterTransmittingPreambleEnergyAccount = J(0);
        transmitterTransmittingHeaderEnergyAccount = J(0);
        transmitterTransmittingDataEnergyAccount = J(0);
        WATCH(totalEnergyAccount);
        WATCH(offEnergyAccount);
        WATCH(sleepEnergyAccount);
        WATCH(switchingEnergyAccount);
        WATCH(receiverIdleEnergyAccount);
        WATCH(receiverBusyEnergyAccount);
        WATCH(receiverReceivingEnergyAccount);
        WATCH(receiverReceivingPreambleEnergyAccount);
        WATCH(receiverReceivingHeaderEnergyAccount);
        WATCH(receiverReceivingDataEnergyAccount);
        WATCH(transmitterIdleEnergyAccount);
        WATCH(transmitterTransmittingEnergyAccount);
        WATCH(transmitterTransmittingPreambleEnergyAccount);
        WATCH(transmitterTransmittingHeaderEnergyAccount);
        WATCH(transmitterTransmittingDataEnergyAccount);

        lastRadioMode = IRadio::RADIO_MODE_OFF;
        lastReceptionState = IRadio::RECEPTION_STATE_UNDEFINED;
        lastTransmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;
        lastReceivedSignalPart = IRadioSignal::SIGNAL_PART_NONE;
        lastTransmittedSignalPart = IRadioSignal::SIGNAL_PART_NONE;

    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT)
        ;
}

void StateBasedMonitoringEpEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signal, long value, cObject *details)
{
    if (signal == IRadio::radioModeChangedSignal ||
        signal == IRadio::receptionStateChangedSignal ||
        signal == IRadio::transmissionStateChangedSignal ||
        signal == IRadio::receivedSignalPartChangedSignal ||
        signal == IRadio::transmittedSignalPartChangedSignal)
    {
        //printf("XXXXX %s \n\tlastRadioMode: %s\t\t\tradioMode: %s \n\tlastReceptionState: %s\t\treceptionState: %s \n\tlastTransmissionState: %s\ttransmissionState: %s \n\tlastReceivedSignalpart: %s\t\treceivedSignalPart: %s \n\tlastTransmittedSignalpart: %s\t\ttransmittedSignalPart: %s \n", this->getParentModule()->getParentModule()->getParentModule()->getName(), radio->getRadioModeName(lastRadioMode), radio->getRadioModeName(radio->getRadioMode()), radio->getRadioReceptionStateName(lastReceptionState), radio->getRadioReceptionStateName(radio->getReceptionState()), radio->getRadioTransmissionStateName(lastTransmissionState), radio->getRadioTransmissionStateName(radio->getTransmissionState()), IRadioSignal::getSignalPartName(lastReceivedSignalPart), IRadioSignal::getSignalPartName(radio->getReceivedSignalPart()), IRadioSignal::getSignalPartName(lastTransmittedSignalPart), IRadioSignal::getSignalPartName(radio->getTransmittedSignalPart()));
        updateEnergyAccounts();
        powerConsumption = computePowerConsumption();
        emit(powerConsumptionChangedSignal, powerConsumption.get());

        lastRadioMode = radio->getRadioMode();
        lastReceptionState = radio->getReceptionState();
        lastTransmissionState = radio->getTransmissionState();
        lastReceivedSignalPart = radio->getReceivedSignalPart();
        lastTransmittedSignalPart = radio->getTransmittedSignalPart();
    }
    else
        throw cRuntimeError("Unknown signal");
}

void StateBasedMonitoringEpEnergyConsumer::updateEnergyAccounts()
{
    if (lastRadioMode == IRadio::RADIO_MODE_OFF){
        calculateEnergy(&offEnergyAccount);
        emit(energyAccountChangedSignal, OFF_ACCOUNT);
    }
    else if (lastRadioMode == IRadio::RADIO_MODE_SLEEP){
        calculateEnergy(&sleepEnergyAccount);
        emit(energyAccountChangedSignal, SLEEP_ACCOUNT);
    }
    else if (lastRadioMode == IRadio::RADIO_MODE_SWITCHING){
        calculateEnergy(&switchingEnergyAccount);
        emit(energyAccountChangedSignal, SWITCHING_ACCOUNT);
    }
    if (lastRadioMode == IRadio::RADIO_MODE_RECEIVER || lastRadioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (lastReceptionState == IRadio::RECEPTION_STATE_IDLE){
            calculateEnergy(&receiverIdleEnergyAccount);
            emit(energyAccountChangedSignal, RECEIVER_IDLE_ACCOUNT);
        }
        else if (lastReceptionState == IRadio::RECEPTION_STATE_BUSY){
            calculateEnergy(&receiverBusyEnergyAccount);
            emit(energyAccountChangedSignal, RECEIVER_BUSY_ACCOUNT);
        }
        else if (lastReceptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_WHOLE){
                calculateEnergy(&receiverReceivingEnergyAccount);
                emit(energyAccountChangedSignal, RECEIVER_RECEIVING_ACCOUNT);
            }
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_PREAMBLE){
                calculateEnergy(&receiverReceivingPreambleEnergyAccount);
                emit(energyAccountChangedSignal, RECEIVER_RECEIVING_PREAMBLE_ACCOUNT);
            }
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_HEADER){
                calculateEnergy(&receiverReceivingHeaderEnergyAccount);
                emit(energyAccountChangedSignal, RECEIVER_RECEIVING_HEADER_ACCOUNT);
            }
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_DATA){
                calculateEnergy(&receiverReceivingDataEnergyAccount);
                emit(energyAccountChangedSignal, RECEIVER_RECEIVING_DATA_ACCOUNT);
            }
            else
                throw cRuntimeError("Unknown received signal part");
        }
        else if (lastReceptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
    }
    if (lastRadioMode == IRadio::RADIO_MODE_TRANSMITTER || lastRadioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (lastTransmissionState == IRadio::TRANSMISSION_STATE_IDLE){
            calculateEnergy(&transmitterIdleEnergyAccount);
            emit(energyAccountChangedSignal, TRANSMITTER_IDLE_ACCOUNT);
        }
        else if (lastTransmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_WHOLE){
                calculateEnergy(&transmitterTransmittingEnergyAccount);
                emit(energyAccountChangedSignal, TRANSMITTER_TRANSMITTING_ACCOUNT);
            }
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_PREAMBLE){
                calculateEnergy(&transmitterTransmittingPreambleEnergyAccount);
                emit(energyAccountChangedSignal, TRANSMITTER_TRANSMITTING_PREAMBLE_ACCOUNT);
            }
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_HEADER){
                calculateEnergy(&transmitterTransmittingHeaderEnergyAccount);
                emit(energyAccountChangedSignal, TRANSMITTER_TRANSMITTING_HEADER_ACCOUNT);
            }
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_DATA){
                calculateEnergy(&transmitterTransmittingDataEnergyAccount);
                emit(energyAccountChangedSignal, TRANSMITTER_TRANSMITTING_DATA_ACCOUNT);
            }
            else
                throw cRuntimeError("Unknown transmitted signal part");
        }
        else if (lastTransmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio transmission state");
    }
}

void StateBasedMonitoringEpEnergyConsumer::calculateEnergy(J* energyAccount)
{
    simtime_t currentSimulationTime = simTime();
    if (currentSimulationTime != lastUpdate) {
        totalEnergyAccount += s((currentSimulationTime - lastUpdate).dbl()) * (powerConsumption);
        *energyAccount += s((currentSimulationTime - lastUpdate).dbl()) * (powerConsumption);
        lastUpdate = currentSimulationTime;
    }
}

double StateBasedMonitoringEpEnergyConsumer::getEnergyFromAccount(EnergyAccount energyAccount)
{
    Enter_Method_Silent();

    double ret;
    switch(energyAccount){
    case TOTAL_ACCOUNT:
        ret = totalEnergyAccount.get();
        break;
    case OFF_ACCOUNT:
        ret = offEnergyAccount.get();
        break;
    case SLEEP_ACCOUNT:
        ret = sleepEnergyAccount.get();
        break;
    case SWITCHING_ACCOUNT:
        ret = switchingEnergyAccount.get();
        break;
    case RECEIVER_IDLE_ACCOUNT:
        ret = receiverIdleEnergyAccount.get();
        break;
    case RECEIVER_BUSY_ACCOUNT:
        ret = receiverBusyEnergyAccount.get();
        break;
    case RECEIVER_RECEIVING_ACCOUNT:
        ret = receiverReceivingEnergyAccount.get();
        break;
    case RECEIVER_RECEIVING_PREAMBLE_ACCOUNT:
        ret = receiverReceivingPreambleEnergyAccount.get();
        break;
    case RECEIVER_RECEIVING_HEADER_ACCOUNT:
        ret = receiverReceivingHeaderEnergyAccount.get();
        break;
    case RECEIVER_RECEIVING_DATA_ACCOUNT:
        ret = receiverReceivingDataEnergyAccount.get();
        break;
    case TRANSMITTER_IDLE_ACCOUNT:
        ret = transmitterIdleEnergyAccount.get();
        break;
    case TRANSMITTER_TRANSMITTING_ACCOUNT:
        ret = transmitterTransmittingEnergyAccount.get();
        break;
    case TRANSMITTER_TRANSMITTING_PREAMBLE_ACCOUNT:
        ret = transmitterTransmittingPreambleEnergyAccount.get();
        break;
    case TRANSMITTER_TRANSMITTING_HEADER_ACCOUNT:
        ret = transmitterTransmittingHeaderEnergyAccount.get();
        break;
    case TRANSMITTER_TRANSMITTING_DATA_ACCOUNT:
        ret = transmitterTransmittingDataEnergyAccount.get();
        break;
    default:
        throw cRuntimeError("StateBasedMonitoringEpEnergyConsumer::getEnergyFromAccount -> unknown Account");
    }

    return ret;
}


} //namespace physicallayer

} //naemspace inet
