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
    if (lastRadioMode == IRadio::RADIO_MODE_OFF)
        calculateEnergy(&offEnergyAccount);
    else if (lastRadioMode == IRadio::RADIO_MODE_SLEEP)
        calculateEnergy(&sleepEnergyAccount);
    else if (lastRadioMode == IRadio::RADIO_MODE_SWITCHING)
        calculateEnergy(&switchingEnergyAccount);
    if (lastRadioMode == IRadio::RADIO_MODE_RECEIVER || lastRadioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (lastReceptionState == IRadio::RECEPTION_STATE_IDLE)
            calculateEnergy(&receiverIdleEnergyAccount);
        else if (lastReceptionState == IRadio::RECEPTION_STATE_BUSY)
            calculateEnergy(&receiverBusyEnergyAccount);
        else if (lastReceptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_WHOLE)
                calculateEnergy(&receiverReceivingEnergyAccount);
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_PREAMBLE)
                calculateEnergy(&receiverReceivingPreambleEnergyAccount);
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_HEADER)
                calculateEnergy(&receiverReceivingHeaderEnergyAccount);
            else if (lastReceivedSignalPart == IRadioSignal::SIGNAL_PART_DATA)
                calculateEnergy(&receiverReceivingDataEnergyAccount);
            else
                throw cRuntimeError("Unknown received signal part");
        }
        else if (lastReceptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
    }
    if (lastRadioMode == IRadio::RADIO_MODE_TRANSMITTER || lastRadioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (lastTransmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            calculateEnergy(&transmitterIdleEnergyAccount);
        else if (lastTransmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_WHOLE)
                calculateEnergy(&transmitterTransmittingEnergyAccount);
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_PREAMBLE)
                calculateEnergy(&transmitterTransmittingPreambleEnergyAccount);
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_HEADER)
                calculateEnergy(&transmitterTransmittingHeaderEnergyAccount);
            else if (lastTransmittedSignalPart == IRadioSignal::SIGNAL_PART_DATA)
                calculateEnergy(&transmitterTransmittingDataEnergyAccount);
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


//W StateBasedMonitoringEpEnergyConsumer::computePowerConsumption() const
//{
//    IRadio::RadioMode radioMode = radio->getRadioMode();
//    if (radioMode == IRadio::RADIO_MODE_OFF){
//        //currentEnergyAccount = &offEnergyAccount;
//        return offPowerConsumption;
//    }
//    else if (radioMode == IRadio::RADIO_MODE_SLEEP){
//        return sleepPowerConsumption;
//    }
//    else if (radioMode == IRadio::RADIO_MODE_SWITCHING){
//        return switchingPowerConsumption;
//    }
//    W powerConsumption = W(0);
//    IRadio::ReceptionState receptionState = radio->getReceptionState();
//    IRadio::TransmissionState transmissionState = radio->getTransmissionState();
//    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
//        if (receptionState == IRadio::RECEPTION_STATE_IDLE){
//            powerConsumption += receiverIdlePowerConsumption;
//        }
//        else if (receptionState == IRadio::RECEPTION_STATE_BUSY){
//            powerConsumption += receiverBusyPowerConsumption;
//        }
//        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING) {
//            auto part = radio->getReceivedSignalPart();
//            if (part == IRadioSignal::SIGNAL_PART_NONE)
//                ;
//            else if (part == IRadioSignal::SIGNAL_PART_WHOLE){
//                powerConsumption += receiverReceivingPowerConsumption;
//            }
//            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE){
//                powerConsumption += receiverReceivingPreamblePowerConsumption;
//            }
//            else if (part == IRadioSignal::SIGNAL_PART_HEADER){
//                powerConsumption += receiverReceivingHeaderPowerConsumption;
//            }
//            else if (part == IRadioSignal::SIGNAL_PART_DATA){
//                powerConsumption += receiverReceivingDataPowerConsumption;
//            }
//            else
//                throw cRuntimeError("Unknown received signal part");
//        }
//        else if (receptionState != IRadio::RECEPTION_STATE_UNDEFINED)
//            throw cRuntimeError("Unknown radio reception state");
//    }
//    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
//        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE){
//            powerConsumption += transmitterIdlePowerConsumption;
//        }
//        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
//            // TODO: add transmission power?
//            auto part = radio->getTransmittedSignalPart();
//            if (part == IRadioSignal::SIGNAL_PART_NONE)
//                ;
//            else if (part == IRadioSignal::SIGNAL_PART_WHOLE){
//                powerConsumption += transmitterTransmittingPowerConsumption;
//            }
//            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE){
//                powerConsumption += transmitterTransmittingPreamblePowerConsumption;
//            }
//            else if (part == IRadioSignal::SIGNAL_PART_HEADER){
//                powerConsumption += transmitterTransmittingHeaderPowerConsumption;
//            }
//            else if (part == IRadioSignal::SIGNAL_PART_DATA){
//                powerConsumption += transmitterTransmittingDataPowerConsumption;
//            }
//            else
//                throw cRuntimeError("Unknown transmitted signal part");
//        }
//        else if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
//            throw cRuntimeError("Unknown radio transmission state");
//    }
//    return powerConsumption;
//}


} //namespace physicallayer

} //naemspace inet
