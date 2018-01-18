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
#include <mono/metadata/assembly.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
#include <mono/utils/mono-publib.h>

#include <stdio.h>
#include <unordered_map>

#include "inet/applications/external/ExternalApp.h"


namespace inet {

/**
 * TODO - Generated class
 */
class ApplicationAdapter : public cSimpleModule
{
  public:
    ApplicationAdapter();
    ~ApplicationAdapter();

    unsigned long createNode();
    void createNode(unsigned long id);
    void send(unsigned long from_id);


  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish() override;


  private:
    MonoDomain* monoDomain;
    MonoAssembly* monoAssembly;
    MonoImage* monoImage;
    MonoClass* monoClass;
    const char* assemblyName;
    const char* namespaceName;
    const char* className;

    uint32 creationCnt;
    std::unordered_map<unsigned long, ExternalApp*> nodeMap; // fast access, slower iteration
    std::map<const char*, MonoMethod*> functionMap = {
            {"initSimulation", NULL},
            {"simulationReady", NULL},
            //{"methodA", NULL},
    };

  private:
    ExternalApp* createNewNode();
    unsigned long getUniqueId();
    void saveNode(unsigned long id, ExternalApp* nodeApp);
    void getExternalFunctioinPtrs(MonoClass* klass);

  public:
    static ApplicationAdapter* instance;
};

/**
 * wrappers to be called from loaded assembly
 */
extern "C" unsigned long aa_createNodeAndId() {return ApplicationAdapter::instance->createNode();}
extern "C" void aa_createNode(unsigned long id) {ApplicationAdapter::instance->createNode(id);}
extern "C" void aa_send(unsigned long from_id) {ApplicationAdapter::instance->send(from_id);}

} //namespace

#endif
