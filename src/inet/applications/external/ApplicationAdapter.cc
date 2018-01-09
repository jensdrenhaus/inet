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

#include "ApplicationAdapter.h"

namespace inet {

Define_Module(ApplicationAdapter);

ApplicationAdapter* ApplicationAdapter::instance = nullptr;

void ApplicationAdapter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        printf("Hello from Application Adapter\n");

        MonoDomain* domain;
        MonoAssembly* assembly;
        MonoImage* image;
        MonoClass* klass;
        MonoObject* obj;
        const char* file;
        int retval;

        /*
         * Load the default Mono configuration file, this is needed
         * if you are planning on using the dllmaps defined on the
         * system configuration
         */
        mono_config_parse (NULL);

        file = par("assemblyName").stringValue();

        /*
         * mono_jit_init() creates a domain: each assembly is
         * loaded and run in a MonoDomain.
         * parameter name is arbitrary
         */
        domain = mono_jit_init (file);

        /*
         * Open the executable
         * This only loads the code, but it will not execute anything yet.
         */
        //assembly = mono_domain_assembly_open (domain, "SadPhyNetFlow.exe");
        assembly = mono_domain_assembly_open (domain, file);
        if (!assembly)
            throw cRuntimeError("%s not found", file);

        /*
         * run the Main() method in the assembly.
         */
        char* dummy = "dummy";
        mono_jit_exec (domain, assembly, 1, &dummy); // treet argc as 1 and argv as file
    }

    if (stage == INITSTAGE_LAST) {
        printf("Hello from Application Adapter again\n");

        /*
         * ####### TEST #######
         */
        // find factory object
        cModuleType *moduleType = cModuleType::get("inet.node.wsn.PhyNode");

        // create (possibly compound) module and build its submodules (if any)
        cModule *module = moduleType->create("node", this->getParentModule());
        module->finalizeParameters();
        module->buildInside();

        // create activation message
        module->scheduleStart(simTime());

        /*
         * ####### END #######
         */
    }

}

void ApplicationAdapter::handleMessage(cMessage *msg)
{
    // no messages expected
}

long ApplicationAdapter::createNode()
{
//    // find factory object
//    const char* moduleTypeName = ApplicationAdapter::instance->par("nodeType").stringValue();
//    cModuleType *moduleType = cModuleType::get(moduleTypeName);
//
//    // create compound module and build its submodules
//    cModule* parentModule = ApplicationAdapter::instance->getParentModule();
//    const char* parentModuleName = parentModule->getFullName();
//    printf("%s", parentModuleName);
//    cModule *newNode = moduleType->create("DynNode", ApplicationAdapter::instance);
//    newNode->finalizeParameters();
//    newNode->buildInside();
//
//    //create activation message
//    newNode->scheduleStart(simTime());



    printf("Node createtd\n");
    return 1;
}

ApplicationAdapter::ApplicationAdapter()
{
    if(ApplicationAdapter::instance == nullptr)
        ApplicationAdapter::instance = this;
    else {
        printf("ERROR : More then one instance of ApplicationAdapter!\n");
        throw cRuntimeError("More then one instance of ApplicationAdapter!");
    }
}

} //namespace
