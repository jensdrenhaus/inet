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

#include "inet/applications/external/ApplicationAdapter.h"

namespace inet {

/**
 * wrappers to be called from loaded assembly
 */
extern "C" unsigned long aa_createNodeAndId() {return ApplicationAdapter::instance->createNode();}
extern "C" void aa_createNode(unsigned long id) {ApplicationAdapter::instance->createNode(id);}
extern "C" void aa_send(unsigned long srcId, unsigned long destId, int numBytes, int msgId) {ApplicationAdapter::instance->send(srcId, destId, numBytes, msgId);}
extern "C" void aa_wait_ms(unsigned long id, int duration) {ApplicationAdapter::instance->wait_ms(id, duration);}
extern "C" void aa_wait_s(unsigned long id, int duration) {ApplicationAdapter::instance->wait_s(id, duration);}
extern "C" void aa_set_global_timer_ms(int duration) {ApplicationAdapter::instance->setGlobalTimer_ms(duration);}
extern "C" void aa_set_global_timer_s(int duration) {ApplicationAdapter::instance->setGlobalTimer_s(duration);}

ApplicationAdapter* ApplicationAdapter::instance = nullptr;

Define_Module(ApplicationAdapter);

/*
 *  ###########################################################################
 *
 *  functions to be called by omnet
 *  ===============================
 *  ###########################################################################
 */

void ApplicationAdapter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        creationCnt = 0;
        instance = this;
        assemblyName = par("assemblyName").stringValue();
        namespaceName = par("namespaceName").stringValue();
        className = par("className").stringValue();

        printf("Application Adapter is loading %s\n", assemblyName);

        /* Load the default Mono configuration file, this is needed
         * if you are planning on using the dllmaps defined on the
         * system configuration
         */
        mono_config_parse (NULL);

        /* mono_jit_init() creates a domain: each assembly is
         * loaded and run in a MonoDomain.
         * parameter name is arbitrary
         */
        monoDomain = mono_jit_init (assemblyName);

        /* Open the executable
         * This only loads the code, but it will not execute anything yet.
         */
        monoAssembly = mono_domain_assembly_open (monoDomain, assemblyName);
        if (!monoAssembly)
            throw cRuntimeError("%s not found", assemblyName);

    }

    if (stage == INITSTAGE_LAST) {

        // run the Main() method in the assembly.
        printf("Application Adapter executes Main() in %s\n", assemblyName);
        char* dummy = "dummy";
        mono_jit_exec (monoDomain, monoAssembly, 1, &dummy); // treet argc as 1 and argv[0] as dummy

        // preparation to get methods from assembly
        printf("Application Adapter is configured to retreve class '%s' in namespace '%s'\n", className, namespaceName);
        monoImage = mono_assembly_get_image (monoAssembly);
        monoClass = mono_class_from_name (monoImage, namespaceName, className);
        if (!monoClass)
            throw cRuntimeError("Can't find Type '%s' in assembly %s\n", className, mono_image_get_filename(monoImage));
        printf("Application Adapter searches for required callback functions \n");
        getExternalFunctioinPtrs(monoClass);
        printf("SUCCESS! Setup of external assembly done \n\n");

        trigger = new cMessage("trigger");
        scheduleAt(SimTime(),trigger); // schedule self message right at the beginning

        call_initSimulation();
    }
}

void ApplicationAdapter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg->getKind() == timer_kind)
            call_globalTimerNotify();
        else if (msg->getKind() == trigger_kind)
            call_simulationReady();
    }
}

void ApplicationAdapter::finish()
{
    call_simulationFinished();
    instance = nullptr;
}

/*
 *  ###########################################################################
 *
 *  functions to be called by external code
 *  =======================================
 *  ###########################################################################
 */

void ApplicationAdapter::send(unsigned long srcId, unsigned long destId, int numBytes, int msgId)
{
    ExternalApp* app = checkNodeId(srcId);
    //app->sendPing();
    app->sendMsg(destId, numBytes, msgId);
}

void ApplicationAdapter::wait_ms(unsigned long id, int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_MS);
    ExternalApp* app = checkNodeId(id);
    app->wait(t);
}

void ApplicationAdapter::wait_s(unsigned long id, int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_S);
    ExternalApp* app = checkNodeId(id);
    app->wait(t);
}

void ApplicationAdapter::setGlobalTimer_ms(int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_MS);
    if (!timer->isScheduled())
        scheduleAt(simTime()+t, timer);
    else
        throw cRuntimeError("timer is already scheduled");
}

void ApplicationAdapter::setGlobalTimer_s(int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_S);
    if (!timer->isScheduled())
            scheduleAt(simTime()+t, timer);
    else
        throw cRuntimeError("timer is already scheduled");
}

void ApplicationAdapter::createNode(unsigned long id)
{
    if (id == 0 || id == 0xFFFFFFFFFFFF)
        throw cRuntimeError("invalid node id!");
    ExternalApp* appPtr = createNewNode(id);
    if (appPtr->getNodeId() != id)
        throw cRuntimeError("error with node id");
    saveNode(id, appPtr);

    const char* name = appPtr->getParentModule()->getName();
    printf("%s created with Id %ld\n", name, id);
}

unsigned long ApplicationAdapter::createNode()
{
    ExternalApp* appPtr = createNewNode(0); // auto generate MAC address
    unsigned long id = appPtr->getNodeId();
    saveNode(id, appPtr);

    const char* name = appPtr->getParentModule()->getName();
    printf("%s created with auto-Id %ld\n", name, id);
}

/*
 *  ###########################################################################
 *
 *  functions to call external code
 *  ===============================
 *  MonoObjekt* mono_runtime_invoke (1,2,3,4)
 *  1) MonoMethod*  method
 *  2) void*        obj   -> NULL if static function. MonoObject if member function
 *  3) void**       param -> array with pointers to arguments
 *  4) MonoObject** exc   -> NULL if no exeption handling
 *  the return value is boxed in the retruned MonoObject*
 *  ###########################################################################
 */

void ApplicationAdapter::call_initSimulation()
{
    MonoMethod* m = checkFunctionPtr("initSimulation");
    mono_runtime_invoke (m, NULL, NULL, NULL);
}

void ApplicationAdapter::call_simulationReady()
{
    MonoMethod* m = checkFunctionPtr("simulationReady");
    mono_runtime_invoke (m, NULL, NULL, NULL);
}

void ApplicationAdapter::call_simulationFinished()
{
    MonoMethod* m = checkFunctionPtr("simulationFinished");
    mono_runtime_invoke (m, NULL, NULL, NULL);
}

void ApplicationAdapter::call_receptionNotify(unsigned long destId, unsigned long srcId, int msgId, int status)
{
    Enter_Method("receptionNotify");

    void* args [4];
    args[0] = &destId;
    args[1] = &srcId;
    args[2] = &msgId;
    args[3] = &status;

    MonoMethod* m = checkFunctionPtr("receptionNotify");
    mono_runtime_invoke (m, NULL, args, NULL);
}

void ApplicationAdapter::call_timerNotify(unsigned long nodeId)
{
    Enter_Method("timerNotify");

    void* args [1];
    args[0] = &nodeId;

    MonoMethod* m = checkFunctionPtr("timerNotify");
    mono_runtime_invoke (m, NULL, args, NULL);
}

void ApplicationAdapter::call_globalTimerNotify()
{
    MonoMethod* m = checkFunctionPtr("globalTimerNotify");
    mono_runtime_invoke (m, NULL, NULL, NULL);
}

/*
 *  ###########################################################################
 *
 *  private functions
 *  =================
 *  ###########################################################################
 */

unsigned long ApplicationAdapter::getUniqueId()
{
    unsigned long id = 0;
    while (id == 0) {
        id = getSimulation()->getUniqueNumber();
    }
    return id;
}

ExternalApp* ApplicationAdapter::createNewNode(unsigned long id)
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

    //manipulate parameter of submodules
    cModule* tempModule = newNode->getSubmodule("nic")->getSubmodule("mac");
    if (id != 0){
        char addrStr[20];
        sprintf(addrStr, "%012lx", id);
        tempModule->par("address") = addrStr;
    }
    else
        tempModule->par("address") = "auto"; // let MAC module create an address

    //initialize
    newNode->callInitialize();

    tempModule = newNode->getSubmodule("app");
    if(!tempModule)
        throw cRuntimeError("Cannot find Submodule 'app' in created Node");
    ExternalApp* appPtr = check_and_cast<ExternalApp*>(tempModule);
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

void ApplicationAdapter::getExternalFunctioinPtrs(MonoClass* klass)
{
    std::map<const char*, MonoMethod*>::iterator iter;

    /*
     * mono_class_get_method_from_name obtains a MonoMethod with a
     * given name. It only works if there are no multiple signatures
     * for any given method name. Last argument is number of parameters. -1 for any number.
     */
    for (iter=functionMap.begin(); iter!=functionMap.end(); ++iter) {
        iter->second = mono_class_get_method_from_name(klass, iter->first, -1);
        if (!iter->second)
            throw cRuntimeError("function '%s' not found in assembly", iter->first);
    }
}

MonoMethod* ApplicationAdapter::checkFunctionPtr(const char* handle)
{
    if (functionMap.find(handle) == functionMap.end())
        throw cRuntimeError("cannot find pointer associated with '%s'", handle);
    else if (functionMap[handle] == NULL)
        throw cRuntimeError("'%s' has no valid function pointer", handle);
    return functionMap[handle];
}

ExternalApp* ApplicationAdapter::checkNodeId(unsigned long handle)
{
    if (nodeMap.find(handle) == nodeMap.end())
        throw cRuntimeError("cannot find node accociatied with Id %ld", handle);
    else if (nodeMap[handle] == NULL)
        throw cRuntimeError("%ld has no valid node pointer", handle);
    return nodeMap[handle];
}

ApplicationAdapter::ApplicationAdapter()
{
    instance = nullptr;
    trigger = nullptr;
    timer = nullptr;
}

ApplicationAdapter::~ApplicationAdapter()
{
    cancelAndDelete(trigger);
    instance == nullptr;
    mono_jit_cleanup (monoDomain);
}

} //namespace
