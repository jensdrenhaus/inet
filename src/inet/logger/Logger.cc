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

namespace inet {

Define_Module(Logger);

void Logger::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // subscribe to signals
        getSimulation()->getSystemModule()->subscribe("receptionEndedIgnoring", this);

        //open result file
        resultFile.open("./log/test_file.csv", ios::out | ios::app);
        if(resultFile.is_open()){
            resultFile << "RunNr, Entity, VarName, Value";
        }
        else
            throw cRuntimeError("Logger: Unable to open the result file! Does the 'log' folder exist?");
    }
}

void Logger::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
{
    resultFile << src->getFullPath().c_str() << "," << getSignalName(id) <<endl;
}

void Logger::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Logger: no messages expected!");
}

void Logger::finish()
{

}

} //namespace
