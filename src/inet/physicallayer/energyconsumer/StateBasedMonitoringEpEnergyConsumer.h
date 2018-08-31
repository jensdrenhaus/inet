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
    simtime_t lastUpdate;
    J offEnergy = J(NaN);
    J sleepEnergy = J(NaN);
    J switchingEnergy = J(NaN);
    J receiverIdleEnergy = J(NaN);
    J receiverBusyEnergy = J(NaN);
    J receiverReceivingEnergy = J(NaN);
    J receiverReceivingPreambleEnergy = J(NaN);
    J receiverReceivingHeaderEnergy = J(NaN);
    J receiverReceivingDataEnergy = J(NaN);
    J transmitterIdleEnergy = J(NaN);
    J transmitterTransmittingEnergy = J(NaN);
    J transmitterTransmittingPreambleEnergy = J(NaN);
    J transmitterTransmittingHeaderEnergy = J(NaN);
    J transmitterTransmittingDataEnergy = J(NaN);
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, long value, cObject *details) override;
};

} //namespace physicallayer

} //namespace inet

#endif
