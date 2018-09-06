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

#ifndef __INET_STATEBASEDMONITORINGEPENERGYCONSUMER_H_
#define __INET_STATEBASEDMONITORINGEPENERGYCONSUMER_H_

#include "inet/physicallayer/energyconsumer/StateBasedEpEnergyConsumer.h"

namespace inet {

namespace physicallayer {

using namespace inet::power;

/**
 * TODO - Generated class
 */
class INET_API StateBasedMonitoringEpEnergyConsumer : public StateBasedEpEnergyConsumer
{
  protected:
    J totalEnergyAccount = J(0);
    J offEnergyAccount = J(0);
    J sleepEnergyAccount = J(0);
    J switchingEnergyAccount = J(0);
    J receiverIdleEnergyAccount = J(0);
    J receiverBusyEnergyAccount = J(0);
    J receiverReceivingEnergyAccount = J(0);
    J receiverReceivingPreambleEnergyAccount = J(0);
    J receiverReceivingHeaderEnergyAccount = J(0);
    J receiverReceivingDataEnergyAccount = J(0);
    J transmitterIdleEnergyAccount = J(0);
    J transmitterTransmittingEnergyAccount = J(0);
    J transmitterTransmittingPreambleEnergyAccount = J(0);
    J transmitterTransmittingHeaderEnergyAccount = J(0);
    J transmitterTransmittingDataEnergyAccount = J(0);

    simtime_t lastUpdate;
    IRadio::RadioMode lastRadioMode;
    IRadio::ReceptionState lastReceptionState;
    IRadio::TransmissionState lastTransmissionState;
    IRadioSignal::SignalPart lastReceivedSignalPart;
    IRadioSignal::SignalPart lastTransmittedSignalPart;
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    //virtual W computePowerConsumption() const override;
    virtual void updateEnergyAccounts();
    virtual void calculateEnergy(J* energyAccount);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, long value, cObject *details) override;

    enum EnergyAccount{
        TOTAL_ACCOUNT = 0,
        OFF_ACCOUNT,
        SLEEP_ACCOUNT,
        SWITCHING_ACCOUNT,
        RECEIVER_IDLE_ACCOUNT,
        RECEIVER_BUSY_ACCOUNT,
        RECEIVER_RECEIVING_ACCOUNT,
        RECEIVER_RECEIVING_PREAMBLE_ACCOUNT,
        RECEIVER_RECEIVING_HEADER_ACCOUNT,
        RECEIVER_RECEIVING_DATA_ACCOUNT,
        TRANSMITTER_IDLE_ACCOUNT,
        TRANSMITTER_TRANSMITTING_ACCOUNT,
        TRANSMITTER_TRANSMITTING_PREAMBLE_ACCOUNT,
        TRANSMITTER_TRANSMITTING_HEADER_ACCOUNT,
        TRANSMITTER_TRANSMITTING_DATA_ACCOUNT,
    };

    virtual double getEnergyFromAccount(EnergyAccount energyAccount);

    static simsignal_t energyAccountChangedSignal;
};

} //namespace physicallayer

} //namespace inet

#endif
