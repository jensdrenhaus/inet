/*
 * DotnetApplicationAdapter.h
 *
 *  Created on: Jul 3, 2018
 *      Author: jens
 */

#ifndef INET_APPLICATIONS_EXTERNAL_DOTNETAPPLICATIONADAPTER_H_
#define INET_APPLICATIONS_EXTERNAL_DOTNETAPPLICATIONADAPTER_H_

#include "inet/common/INETDefs.h"

#include <string>
#include <unordered_map>

#include "inet/applications/external/ApplicationAdapterBase.h"
#include "inet/applications/external/coreclrhost.h"
#include "inet/applications/external/ExternalAppTrampoline.h"
#include "inet/applications/external/util/NodeFactory.h"
#include "inet/applications/external/ApplicationAdapterReceptionStates.h"


namespace inet {

/**
 * TODO - Generated class
 */

class DotnetApplicationAdapter : public ApplicationAdapterBase
{
    // called from external assembly via wrapper functions
  public:
    unsigned long createNode(ExternalAppTrampoline::NodeType type = ExternalAppTrampoline::UNDEFINED);
    void createNode(unsigned long id, ExternalAppTrampoline::NodeType type = ExternalAppTrampoline::UNDEFINED);
    void send(unsigned long srcId, unsigned long destId, int numBytes, int msgId);
    void wait_ms(unsigned long id, int duration);
    void wait_s(unsigned long id, int duration);
    void setGlobalTimer_s(int duration);
    void setGlobalTimer_ms(int duration);
    unsigned long getGlobalTime();



    // to be called by omnet core
  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish() override;

    // to call external code
  private:
    void callMethod();
    std::map<const char*, void*> functionMap = {       // fast iteration, slower access
                {"initSimulation", NULL},
                {"simulationReady", NULL},
                {"receptionNotify", NULL},
                {"timerNotify", NULL},
                {"globalTimerNotify", NULL},
                {"simulationFinished", NULL},
    };
    void call_initSimulation();
    typedef void (*initSimulation_fptr)(void);
    void call_simulationReady();
    typedef void (*simulationReady_fptr)(void);
    void call_simulationFinished();
    typedef void (*simulationFinished_fptr)(void);
    void call_globalTimerNotify();
    typedef void (*globalTimerNotify_fptr)(void);
  public:
    void call_receptionNotify(unsigned long destId, unsigned long srcId, int msgId, int status);
    typedef void (*receptionNotify_fptr)(unsigned long, unsigned long, int, int);
    void call_timerNotify(unsigned long nodeId);
    typedef void (*timerNotify_fptr)(unsigned long);


  private:
    void* hostHandle;
    unsigned int domainId;
    void* coreclrLib;
    const char* assembly;
    const char* namespaceName;
    const char* className;
    const char* clrFilesPath;

    std::string currentProcessAbsolutePath;
    std::string clrFilesAbsolutePath;
    std::string managedAssemblyAbsolutePath;
    std::string tpaList;

    coreclr_initialize_ptr initializeCoreCLR;
    coreclr_execute_assembly_ptr executeAssembly;
    coreclr_create_delegate_ptr createDelegate;
    coreclr_shutdown_ptr shutdownCoreCLR;

    NodeFactory* factory;
    uint32 creationCnt;
    cMessage* trigger;
    cMessage* timer;
    enum{trigger_kind=1, timer_kind=2};
    std::unordered_map<unsigned long, ExternalAppTrampoline*> nodeMap; // fast access, slower iteration

    // runtime helper
  private:
    void getExternalFunctionPtr(const char* funcName, void** ptr);
    void checkExternalFunctionPtrs();
    bool generateAbsolutePaths(const char* managedAssemblyPath,const char* clrFilesPath);
    int  setupRuntime();
    void shutdownRuntime();
    bool getCoreClrHostInterface(const char* coreClrDll);
    int  executeManagedAssemblyMain(int managedAssemblyArgc, const char** managedAssemblyArgv);
    bool getAbsolutePath(const char* path, std::string& absolutePath);
    bool getDirectory(const char* absolutePath, std::string& directory);
    const char* GetEnvValueBoolean(const char* envVariable);
    void addFilesFromDirectoryToTpaList(const char* directory);

    //factory helper
  private:
    unsigned long getUniqueId();
    void addNodeToList(unsigned long id, ExternalAppTrampoline* nodeApp);
    ExternalAppTrampoline* findNode(unsigned long handle);

  public:
    DotnetApplicationAdapter();
    ~DotnetApplicationAdapter();
    static DotnetApplicationAdapter* instance;
};

} //namespace

#endif /* INET_APPLICATIONS_EXTERNAL_DOTNETAPPLICATIONADAPTER_H_ */
