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

ApplicationAdapter* ApplicationAdapter::instance = nullptr;

Define_Module(ApplicationAdapter);

void ApplicationAdapter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        printf("Hello from Application Adapter\n");
        printf("size of int = %d bytes \nsize of long = %d bytes\n", (int)sizeof(int), (int)sizeof(long));
        creationCnt = 0;
        instance = this;
        assemblyName = par("assemblyName").stringValue();
        namespaceName = par("namespaceName").stringValue();
        className = par("className").stringValue();


        /*
         * Load the default Mono configuration file, this is needed
         * if you are planning on using the dllmaps defined on the
         * system configuration
         */
        mono_config_parse (NULL);

        /*
         * mono_jit_init() creates a domain: each assembly is
         * loaded and run in a MonoDomain.
         * parameter name is arbitrary
         */
        monoDomain = mono_jit_init (assemblyName);

        /*
         * Open the executable
         * This only loads the code, but it will not execute anything yet.
         */
        //assembly = mono_domain_assembly_open (domain, "SadPhyNetFlow.exe");
        monoAssembly = mono_domain_assembly_open (monoDomain, assemblyName);
        if (!monoAssembly)
            throw cRuntimeError("%s not found", par("assemblyName").stringValue());

    }

    if (stage == INITSTAGE_LAST) {

        /*
         * run the Main() method in the assembly.
         */
        char* dummy = "dummy";
        mono_jit_exec (monoDomain, monoAssembly, 1, &dummy); // treet argc as 1 and argv[0] as dummy

        /*
         * get methods from assembly
         */
        monoImage = mono_assembly_get_image (monoAssembly);
        monoClass = mono_class_from_name (monoImage, namespaceName, className);
        if (!monoClass) {
            //fprintf (stderr, "Can't find Type in assembly %s\n", mono_image_get_filename (image));
            throw cRuntimeError("Can't find Type '%s' in assembly %s\n", className, mono_image_get_filename(monoImage));
        }
        getExternalMethods(monoClass);
    }
}

void ApplicationAdapter::handleMessage(cMessage *msg)
{
    // no messages expected
}

void ApplicationAdapter::send(unsigned long from_id)
{
    // TODO
}

unsigned long ApplicationAdapter::createNode()
{
    unsigned long id = getUniqueId();
    createNode(id);
    return id;
}

void ApplicationAdapter::createNode(unsigned long id)
{
    ExternalApp* appPtr = createNewNode();
    saveNode(id, appPtr);
    const char* name = appPtr->getParentModule()->getName();
    printf("%s created with Id %ld\n", name, id);
}

unsigned long ApplicationAdapter::getUniqueId()
{
    return getSimulation()->getUniqueNumber();
}

ExternalApp* ApplicationAdapter::createNewNode()
{
    creationCnt++;
    const char* spacer = (creationCnt < 10) ? "000" : (creationCnt < 100) ? "00" : (creationCnt < 1000) ? "0" : "";;
    char newNodeName[16];
    sprintf(newNodeName, "node%s%d", spacer, creationCnt);

    // find factory object
    const char* moduleTypeName = par("nodeType").stringValue();
    cModuleType *moduleType = cModuleType::get(moduleTypeName);

    // create compound module and build its submodules
    cModule* parentModule = getParentModule(); // parent of nodes must be the network
    cModule *newNode = moduleType->create(newNodeName, parentModule);
    newNode->finalizeParameters();
    newNode->buildInside();

    //create activation message and initialize
    //newNode->scheduleStart(simTime());
    newNode->callInitialize();

    cModule* module = newNode->getSubmodule("app");
    if(!module)
        throw cRuntimeError("Cannot find Submodule 'app' in created Node");
    ExternalApp* appPtr = check_and_cast<ExternalApp*>(module);
    return appPtr;
}

void ApplicationAdapter::saveNode(unsigned long id, ExternalApp* nodeApp)
{
    using namespace std;
    pair<unordered_map<unsigned long, ExternalApp*>::iterator, bool> retVal;
    retVal = nodeMap.insert(pair<unsigned long,ExternalApp*>(id, nodeApp));
    if(retVal.second == false)
        throw cRuntimeError("Node with Id %d already exists", id);
}

void ApplicationAdapter::getExternalMethods(MonoClass* klass)
{
    MonoMethod* m = NULL;
    MonoMethod* methodA = NULL;
    MonoMethod* methodB = NULL;
    MonoProperty* prop;
    MonoObject* result;
    void* it;
    void* args[1];
    int val;

    /* retrieve all the methods we need */
    it = NULL;
    while (m = mono_class_get_methods (klass, &it)) {
        if (strcmp (mono_method_get_name (m), "initSimulation") == 0) {
            methodA = m;
        } else if (strcmp (mono_method_get_name (m), "another_method") == 0) {
            methodB = m;
        }
    }

    /* Now we'll call method (): since it takes no arguments
     * we can pass NULL as the third argument
     * and since it is a static method we don't need to pass
     * an object to mono_runtime_invoke ().
     * The method will print the updated value.
     */
    mono_runtime_invoke (methodA, NULL, NULL, NULL);
}

void ApplicationAdapter::finish()
{
    instance = nullptr;
}

ApplicationAdapter::ApplicationAdapter()
{
    instance = nullptr;
}

ApplicationAdapter::~ApplicationAdapter()
{
    instance == nullptr;
    mono_jit_cleanup (monoDomain);
}

} //namespace
