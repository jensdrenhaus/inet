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
        ;
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
        powerConsumption = computePowerConsumption();
        emit(powerConsumptionChangedSignal, powerConsumption.get());
    }
    else
        throw cRuntimeError("Unknown signal");
}

} //namespace physicallayer

} //naemspace inet
