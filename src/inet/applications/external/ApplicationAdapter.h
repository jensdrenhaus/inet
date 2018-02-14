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

#ifndef __INET_APPLICATIONADAPTER_H_
#define __INET_APPLICATIONADAPTER_H_

#include "inet/common/INETDefs.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>    // mono_assembly_... only
#include <mono/metadata/environment.h> // exitcode_get
#include <mono/metadata/mono-config.h> // mono_config_parse ..
#include <mono/utils/mono-publib.h>  // minimal genaral purpose header for use in public mono headers

#include <stdio.h>
#include <unordered_map>

#include "inet/applications/external/ExternalApp.h"


namespace inet {

/**
 * TODO - Generated class
 */

class ApplicationAdapter : public cSimpleModule
{
    // to be called by wrapper functions
  public:
    unsigned long createNode();
    void createNode(unsigned long id);
    void send(unsigned long srcId, unsigned long destId, int numBytes, int msgId);
    void wait_ms(unsigned long id, int duration);
    void wait_s(unsigned long id, int duration);

    // to be called by omnet core
  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish() override;

    // to call external code
  private:
    std::map<const char*, MonoMethod*> functionMap = {       // fast iteration, slower access
            {"initSimulation", NULL},
            {"simulationReady", NULL},
            {"receptionNotify", NULL},
            {"timerNotify", NULL},
            {"simulationFinished", NULL},
    };
    void call_initSimulation();
    void call_simulationReady();
    void call_simulationFinished();
  public:
    void call_receptionNotify(unsigned long destId, unsigned long srcId, int msgId, int status);
    void call_timerNotify(unsigned long nodeId);


  private:
    MonoDomain* monoDomain;
    MonoAssembly* monoAssembly;
    MonoImage* monoImage;
    MonoClass* monoClass;
    const char* assemblyName;
    const char* namespaceName;
    const char* className;

    uint32 creationCnt;
    cMessage* trigger;
    std::unordered_map<unsigned long, ExternalApp*> nodeMap; // fast access, slower iteration

  private:
    ExternalApp* createNewNode(unsigned long id);
    unsigned long getUniqueId();
    void saveNode(unsigned long id, ExternalApp* nodeApp);
    void getExternalFunctioinPtrs(MonoClass* klass);
    MonoMethod* checkFunctionPtr(const char* handle);
    ExternalApp* checkNodeId(unsigned long handle);

  public:
    ApplicationAdapter();
    ~ApplicationAdapter();
    static ApplicationAdapter* instance;
};


} //namespace

#endif // __INET_APPLICATIONADAPTER_H_
