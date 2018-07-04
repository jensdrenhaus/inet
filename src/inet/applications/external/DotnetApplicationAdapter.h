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

#include "inet/applications/external/coreclrhost.h"

namespace inet {

/**
 * TODO - Generated class
 */

class DotnetApplicationAdapter : public cSimpleModule
{
  public:
    DotnetApplicationAdapter();
    ~DotnetApplicationAdapter();
    void callMethod();

    // to be called by omnet core
  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish() override;

  private:
    bool generateAbsolutePaths(
                            const char* managedAssemblyPath,
                            const char* clrFilesPath);
    int  setupRuntime();
    bool getCoreClrHostInterface(const char* coreClrDll);
    int  executeManagedAssemblyMain(int managedAssemblyArgc, const char** managedAssemblyArgv);
    bool getAbsolutePath(const char* path, std::string& absolutePath);
    bool getDirectory(const char* absolutePath, std::string& directory);
    const char* GetEnvValueBoolean(const char* envVariable);
    void addFilesFromDirectoryToTpaList(const char* directory);

  private:
    void* hostHandle;
    unsigned int domainId;
    void* coreclrLib;

    std::string currentProcessAbsolutePath;
    std::string clrFilesAbsolutePath;
    std::string managedAssemblyAbsolutePath;
    std::string tpaList;

    coreclr_initialize_ptr initializeCoreCLR;
    coreclr_execute_assembly_ptr executeAssembly;
    coreclr_create_delegate_ptr createDelegate;
    coreclr_shutdown_ptr shutdownCoreCLR;

};

} //namespace

#endif /* INET_APPLICATIONS_EXTERNAL_DOTNETAPPLICATIONADAPTER_H_ */
