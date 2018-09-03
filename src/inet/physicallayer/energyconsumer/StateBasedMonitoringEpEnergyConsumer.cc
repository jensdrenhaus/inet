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
        updateEnergyAccounts();
        powerConsumption = computePowerConsumption();
        emit(powerConsumptionChangedSignal, powerConsumption.get());
    }
    else
        throw cRuntimeError("Unknown signal");
}

void StateBasedMonitoringEpEnergyConsumer::updateEnergyAccounts()
{
    IRadio::RadioMode radioMode = radio->getRadioMode();
    if (radioMode == IRadio::RADIO_MODE_OFF)
        currentEnergyAccount = &offEnergyAccount;
    else if (radioMode == IRadio::RADIO_MODE_SLEEP)
        currentEnergyAccount = &offEnergyAccount;
    else if (radioMode == IRadio::RADIO_MODE_SWITCHING)
        currentEnergyAccount = &offEnergyAccount;
    IRadio::ReceptionState receptionState = radio->getReceptionState();
    IRadio::TransmissionState transmissionState = radio->getTransmissionState();
    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (receptionState == IRadio::RECEPTION_STATE_IDLE)
            currentEnergyAccount = &offEnergyAccount;
        else if (receptionState == IRadio::RECEPTION_STATE_BUSY)
            currentEnergyAccount = &offEnergyAccount;
        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            auto part = radio->getReceivedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE)
                currentEnergyAccount = &offEnergyAccount;
            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
                currentEnergyAccount = &offEnergyAccount;
            else if (part == IRadioSignal::SIGNAL_PART_HEADER)
                currentEnergyAccount = &offEnergyAccount;
            else if (part == IRadioSignal::SIGNAL_PART_DATA)
                currentEnergyAccount = &offEnergyAccount;
            else
                throw cRuntimeError("Unknown received signal part");
        }
        else if (receptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
    }
    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            currentEnergyAccount = &offEnergyAccount;
        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            // TODO: add transmission power?
            auto part = radio->getTransmittedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE)
                currentEnergyAccount = &offEnergyAccount;
            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
                currentEnergyAccount = &offEnergyAccount;
            else if (part == IRadioSignal::SIGNAL_PART_HEADER)
                currentEnergyAccount = &offEnergyAccount;
            else if (part == IRadioSignal::SIGNAL_PART_DATA)
                currentEnergyAccount = &offEnergyAccount;
            else
                throw cRuntimeError("Unknown transmitted signal part");
        }
        else if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio transmission state");
    }

    currentEnergyAccount = &offEnergyAccount;
    simtime_t currentSimulationTime = simTime();
    if (currentSimulationTime != lastUpdate) {
        totalEnergyAccount += s((currentSimulationTime - lastUpdate).dbl()) * (powerConsumption);
        *currentEnergyAccount += s((currentSimulationTime - lastUpdate).dbl()) * (powerConsumption);
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
