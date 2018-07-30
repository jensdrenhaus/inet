#include "inet/applications/external/DotnetApplicationAdapter.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <cstring>
#include <dlfcn.h>
#include <cstdlib>
#include <dirent.h>
#include <set>

#include "inet/applications/external/assert.h"

#ifndef SUCCEEDED
#define SUCCEEDED(Status) ((Status) >= 0)
#endif // !SUCCEEDED

namespace inet {

static const char* const relativePathToDll = "../../src/csharp/bin/Debug/netcoreapp2.0/publish/";
static const char* const symlinkEntrypointExecutable = "/proc/self/exe";
static const char* const coreClrDll = "libcoreclr.so";
// Name of the environment variable controlling server GC.
// If set to 1, server GC is enabled on startup. If 0, server GC is
// disabled. Server GC is off by default.
static const char* const serverGcVar = "COMPlus_gcServer";
// Name of environment variable to control "System.Globalization.Invariant"
// Set to 1 for Globalization Invariant mode to be true. Default is false.
static const char* const globalizationInvariantVar = "CORECLR_GLOBAL_INVARIANT";

/**
 * wrappers to be called from loaded assembly
 */
extern "C" void test(char* text) { printf("%s \n", text);}
extern "C" unsigned long daa_createNodeAndId() {return DotnetApplicationAdapter::instance->createNode();}
extern "C" void daa_createNode(unsigned long id) {DotnetApplicationAdapter::instance->createNode(id);}
extern "C" void daa_send(unsigned long srcId, unsigned long destId, int numBytes, int msgId) {DotnetApplicationAdapter::instance->send(srcId, destId, numBytes, msgId);}
extern "C" void daa_wait_ms(unsigned long id, int duration) {DotnetApplicationAdapter::instance->wait_ms(id, duration);}
extern "C" void daa_wait_s(unsigned long id, int duration) {DotnetApplicationAdapter::instance->wait_s(id, duration);}
extern "C" void daa_set_global_timer_ms(int duration) {DotnetApplicationAdapter::instance->setGlobalTimer_ms(duration);}
extern "C" void daa_set_global_timer_s(int duration) {DotnetApplicationAdapter::instance->setGlobalTimer_s(duration);}
extern "C" unsigned long daa_getGlobalTime() {return DotnetApplicationAdapter::instance->getGlobalTime();}

DotnetApplicationAdapter* DotnetApplicationAdapter::instance = nullptr;

Define_Module(DotnetApplicationAdapter);

/*
 *  ###########################################################################
 *
 *  functions called by omnet
 *  ===============================
 *  ###########################################################################
 */

void DotnetApplicationAdapter::initialize(int stage)
{
    ApplicationAdapterBase::initialize(stage);

    char managedAssemblyPath [PATH_MAX];

    if (stage == INITSTAGE_LOCAL) {

        creationCnt = 0;
        factory = new NodeFactory(par("nodeType").stringValue(), this->getParentModule());
        instance = this;
        assemblyName = par("assemblyName").stringValue();
        namespaceName = par("namespaceName").stringValue();
        className = par("className").stringValue();
        clrFilesPath = par("clrFilesPath").stringValue();

        coreclrLib      = NULL;
        hostHandle      = NULL;
        domainId = 0;

        initializeCoreCLR = NULL;
        executeAssembly   = NULL;
        createDelegate    = NULL;
        shutdownCoreCLR   = NULL;

        strcpy(managedAssemblyPath, relativePathToDll);
        strcat(managedAssemblyPath, assemblyName);
        if(!generateAbsolutePaths(managedAssemblyPath, clrFilesPath)){
            throw cRuntimeError("Invalid paths! Check the following relative paths:\n   %s\n   %s", managedAssemblyPath, clrFilesPath);
        }
        printf("Application Adapter is loading %s\n", managedAssemblyAbsolutePath.c_str());

        int exitCode = setupRuntime();
    }

    if (stage == INITSTAGE_LAST) {

        // run the Main() method in the assembly.
        printf("Application Adapter executes Main() in %s\n", assemblyName);
        int exitCode = executeManagedAssemblyMain(0, NULL);


        printf("Application Adapter searches for required callback functions \n");
        checkExternalFunctionPtrs();
        printf("SUCCESS! Setup of external assembly done \n\n");

        timer = new cMessage("timer");
        timer->setKind(timer_kind);

        trigger = new cMessage("trigger");
        trigger->setKind(trigger_kind);
        scheduleAt(SimTime(),trigger); // schedule self message right at the beginning


        call_initSimulation();
        //callMethod();
    }
}

void DotnetApplicationAdapter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg->getKind() == timer_kind)
            call_globalTimerNotify();
        if (msg->getKind() == trigger_kind) {
            delete(msg);
            call_simulationReady();
        }
    }
    else
        throw cRuntimeError("ApplicationAdapter does not expect messages from other modules!");
}

void DotnetApplicationAdapter::finish()
{
    call_simulationFinished();
    shutdownRuntime();
    instance = nullptr;
}

/*
 *  ###########################################################################
 *
 *  functions to be called by external code
 *  =======================================
 *  ###########################################################################
 */

void DotnetApplicationAdapter::send(unsigned long srcId, unsigned long destId, int numBytes, int msgId)
{
    ExternalAppTrampoline* app = findNode(srcId);
    //app->sendPing();
    app->sendMsg(destId, numBytes, msgId);
}

void DotnetApplicationAdapter::wait_ms(unsigned long id, int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_MS);
    ExternalAppTrampoline* app = findNode(id);
    app->wait(t);
}

void DotnetApplicationAdapter::wait_s(unsigned long id, int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_S);
    ExternalAppTrampoline* app = findNode(id);
    app->wait(t);
}

void DotnetApplicationAdapter::setGlobalTimer_ms(int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_MS);
    if (!timer->isScheduled())
        scheduleAt(simTime()+t, timer);
    else
        throw cRuntimeError("timer is already scheduled");
}

void DotnetApplicationAdapter::setGlobalTimer_s(int duration)
{
    simtime_t t = SimTime(duration, SIMTIME_S);
    if (!timer->isScheduled()){
        cancelEvent(timer);
        scheduleAt(simTime()+t, timer);
    }
    else
        throw cRuntimeError("timer is already scheduled");
}

unsigned long DotnetApplicationAdapter::getGlobalTime()
{
    simtime_t time = simTime();
    int scaleExpo = time.getScaleExp();
    int64_t raw = time.raw();
    //printf(" time string     : %s \n time raw        : %ld \n time scale expo : %i \n", time.str().c_str(), raw, scaleExpo);
    if (scaleExpo == -12 && raw >= 0) {//picosec
        return raw;
    }
    else
        throw cRuntimeError("Error in get GlobalTime: unsupported time format");
}

void DotnetApplicationAdapter::createNode(unsigned long id)
{
    if (id == 0 || id == 0xFFFFFFFFFFFF)
        throw cRuntimeError("invalid node id %x!", id);
    ExternalAppTrampoline* appPtr = factory->getNode(id);
    if (appPtr->getNodeId() != id)
        throw cRuntimeError("error with node id");
    addNodeToList(id, appPtr);

    const char* name = appPtr->getParentModule()->getName();
    printf("%s created with Id %ld\n", name, id);
}

unsigned long DotnetApplicationAdapter::createNode()
{
    ExternalAppTrampoline* appPtr = factory->getNode(0); // auto generate MAC address
    unsigned long id = appPtr->getNodeId();
    addNodeToList(id, appPtr);

    const char* name = appPtr->getParentModule()->getName();
    printf("%s created with auto-Id %ld\n", name, id);
}

/*
 *  ###########################################################################
 *
 *  functions to call external code
 *  ===============================
 *  TODO describe parameter exchange
 *  ###########################################################################
 */

void DotnetApplicationAdapter::call_initSimulation()
{
    static initSimulation_fptr delegate = nullptr;
    if (delegate == nullptr)
        getExternalFunctionPtr("initSimulation", (void**)&delegate);
    delegate();
}

void DotnetApplicationAdapter::call_simulationReady()
{
    static simulationReady_fptr delegate = nullptr;
    if (delegate == nullptr)
        getExternalFunctionPtr("simulationReady", (void**)&delegate);
    delegate();
}

void DotnetApplicationAdapter::call_simulationFinished()
{
    static simulationFinished_fptr delegate = nullptr;
    if (delegate == nullptr)
        getExternalFunctionPtr("simulationFinished", (void**)&delegate);
    delegate();
}

void DotnetApplicationAdapter::call_receptionNotify(unsigned long destId, unsigned long srcId, int msgId, int status)
{
    static receptionNotify_fptr delegate = nullptr;
    if (delegate == nullptr)
        getExternalFunctionPtr("receptionNotify", (void**)&delegate);
    delegate(destId, srcId, msgId, status);
}

void DotnetApplicationAdapter::call_timerNotify(unsigned long nodeId)
{
    static timerNotify_fptr delegate = nullptr;
    if (delegate == nullptr)
        getExternalFunctionPtr("timerNotify", (void**)&delegate);
    delegate(nodeId);
}

void DotnetApplicationAdapter::call_globalTimerNotify()
{
    static globalTimerNotify_fptr delegate = nullptr;
    if (delegate == nullptr)
        getExternalFunctionPtr("globalTimerNotify", (void**)&delegate);
    delegate();
}

/*
 *  ###########################################################################
 *
 *  private factory helper functions
 *  =================
 *  ###########################################################################
 */

unsigned long DotnetApplicationAdapter::getUniqueId()
{
    unsigned long id = 0;
    while (id == 0) {
        id = getSimulation()->getUniqueNumber();
    }
    return id;
}

void DotnetApplicationAdapter::addNodeToList(unsigned long id, ExternalAppTrampoline* nodeApp)
{
    using namespace std;
    pair<unordered_map<unsigned long, ExternalAppTrampoline*>::iterator, bool> retVal;
    retVal = nodeMap.insert(pair<unsigned long,ExternalAppTrampoline*>(id, nodeApp));
    if(retVal.second == false)
        throw cRuntimeError("Node with Id %d already exists", id);
}

ExternalAppTrampoline* DotnetApplicationAdapter::findNode(unsigned long handle)
{
    if (nodeMap.find(handle) == nodeMap.end())
        throw cRuntimeError("cannot find node accociatied with Id %ld", handle);
    else if (nodeMap[handle] == NULL)
        throw cRuntimeError("%ld has no valid node pointer", handle);
    return nodeMap[handle];
}

/*
 *  ###########################################################################
 *
 *  private runtime helper functions
 *  =================
 *  ###########################################################################
 */
void DotnetApplicationAdapter::callMethod()
{
    typedef void (*method_ptr)(int);
    method_ptr delegate = nullptr;
    int st = createDelegate(
            hostHandle,
            domainId,
            "myApp", // assembly name,
            "OmnetServices.OmnetInterface", // namespace.class
            "Method", // function name
            (void**)&delegate);

    delegate(42);
}


void DotnetApplicationAdapter::getExternalFunctionPtr(const char* funcName, void** ptr)
{
    int status = createDelegate(hostHandle,
                                domainId,
                                "SampleApp", // assembly name,
                                "OmnetServices.OmnetInterface", // namespace.class
                                funcName, // function name
                                ptr);
    if (status < 0)
        throw cRuntimeError("function '%s' not found in external assembly", funcName);
}

void DotnetApplicationAdapter::checkExternalFunctionPtrs()
{
    int status;
    std::map<const char*, void*>::iterator iter;

    for (iter=functionMap.begin(); iter!=functionMap.end(); ++iter) {
        status = createDelegate(hostHandle,
                                domainId,
                                "SampleApp", // assembly name,
                                "OmnetServices.OmnetInterface", // namespace.class
                                iter->first, // function name
                                &(iter->second));
        if (status < 0)
            throw cRuntimeError("function '%s' not found in external assembly", iter->first);
    }
}

bool DotnetApplicationAdapter::generateAbsolutePaths(const char* managedAssemblyPath, const char* clrFilesPath)
{
//    char cwd[1024];
//    if (getcwd(cwd, sizeof(cwd)) != NULL)
//       printf("Current working dir: %s\n", cwd);
//    else
//        printf("getcwd() error");
    // Check if the specified managed assembly file exists
    struct stat sb;
    if (stat(managedAssemblyPath, &sb) == -1){
        printf("Managed assembly not found: %s\n", managedAssemblyPath);
        return false;
    }

    // Verify that the managed assembly path points to a file
    if (!S_ISREG(sb.st_mode)){
        printf("The specified managed assembly is not a file\n");
        return false;
    }
    // // Get managed assembly absolute path
    if (!getAbsolutePath(managedAssemblyPath, managedAssemblyAbsolutePath)){
        printf("Failed to convert managed assembly path to absolute path\n");
        return false;
    }

    // Get path to the executable for the current process.
    // On linux, return the symlink that will be resolved by GetAbsolutePath
    // to fetch the entrypoint EXE absolute path, inclusive of filename.
    currentProcessAbsolutePath.clear();

    if (!getAbsolutePath(symlinkEntrypointExecutable, currentProcessAbsolutePath)){
        printf("Could not get full path of current process");
        return false;
    }

    // Get CLR files absolute path
    std::string clrFilesRelativePath;
    const char* clrFilesPathLocal = clrFilesPath;
    if (clrFilesPathLocal == NULL){
        // There was no CLR files path specified, use the folder of the current process
        if (!getDirectory(currentProcessAbsolutePath.c_str(), clrFilesRelativePath)){
            printf("Failed to get directory from current process \n");
            return false;
        }

        clrFilesPathLocal = clrFilesRelativePath.c_str();

        // TODO: consider using an env variable (if defined) as a fall-back.
        // The windows version of the corerun uses core_root env variable
    }

    if (!getAbsolutePath(clrFilesPathLocal, clrFilesAbsolutePath)){
        perror("Failed to convert CLR files path to absolute path");
        return false;
    }

    return true;
}

int DotnetApplicationAdapter::setupRuntime()
{
    int exitCode = -1;
    std::string appFolder;
    getDirectory(managedAssemblyAbsolutePath.c_str(), appFolder);
    std::string coreClrDllPath(clrFilesAbsolutePath);
    coreClrDllPath.append("/");
    coreClrDllPath.append(coreClrDll);

    if (coreClrDllPath.length() >= PATH_MAX){
        printf("Absolute path to libcoreclr.so too long\n");
        return -1;
    }

    // Construct native search directory paths
    // and TPA list

    if (strlen(managedAssemblyAbsolutePath.c_str()) > 0)
    {
        // Target assembly should be added to the tpa list. Otherwise corerun.exe
        // may find wrong assembly to execute.
        // Details can be found at https://github.com/dotnet/coreclr/issues/5631
        tpaList = managedAssemblyAbsolutePath;
        tpaList.append(":");
    }

    std::string nativeDllSearchDirs(appFolder);
    // char *coreLibraries = getenv("CORE_LIBRARIES");
    // if (coreLibraries)
    // {
    //     nativeDllSearchDirs.append(":");
    //     nativeDllSearchDirs.append(coreLibraries);
    //     if (std::strcmp(coreLibraries, clrFilesAbsolutePath) != 0)
    //     {
    //         AddFilesFromDirectoryToTpaList(coreLibraries, tpaList);
    //     }
    // }
    nativeDllSearchDirs.append(":");
    nativeDllSearchDirs.append(clrFilesAbsolutePath);
    addFilesFromDirectoryToTpaList(clrFilesAbsolutePath.c_str());

    std::string currentProcessDir("");
    getDirectory(currentProcessAbsolutePath.c_str(), currentProcessDir);
    nativeDllSearchDirs.append(":");
    nativeDllSearchDirs.append(currentProcessDir);
    addFilesFromDirectoryToTpaList(currentProcessDir.c_str());

    //
    // create a host and an app domain
    //
    if (getCoreClrHostInterface(coreClrDllPath.c_str())){

        // Check whether we are enabling server GC (off by default)
        const char* useServerGc = GetEnvValueBoolean(serverGcVar);

        // Check Globalization Invariant mode (false by default)
        const char* globalizationInvariant = GetEnvValueBoolean(globalizationInvariantVar);

        // Allowed property names:
        // APPBASE
        // - The base path of the application from which the exe and other assemblies will be loaded
        //
        // TRUSTED_PLATFORM_ASSEMBLIES
        // - The list of complete paths to each of the fully trusted assemblies
        //
        // APP_PATHS
        // - The list of paths which will be probed by the assembly loader
        //
        // APP_NI_PATHS
        // - The list of additional paths that the assembly loader will probe for ngen images
        //
        // NATIVE_DLL_SEARCH_DIRECTORIES
        // - The list of paths that will be probed for native DLLs called by PInvoke
        //
        const char *propertyKeys[] = {
            "TRUSTED_PLATFORM_ASSEMBLIES",
            "APP_PATHS",
            "APP_NI_PATHS",
            "NATIVE_DLL_SEARCH_DIRECTORIES",
            "System.GC.Server",
            "System.Globalization.Invariant",
        };
        const char *propertyValues[] = {
            // TRUSTED_PLATFORM_ASSEMBLIES
            tpaList.c_str(),
            // APP_PATHS
            appFolder.c_str(),
            // APP_NI_PATHS
            appFolder.c_str(),
            // NATIVE_DLL_SEARCH_DIRECTORIES
            nativeDllSearchDirs.c_str(),
            // System.GC.Server
            useServerGc,
            // System.Globalization.Invariant
            globalizationInvariant,
        };

        //printf("nativeDllSearchDirs: %s \n", nativeDllSearchDirs.c_str());

        int st = initializeCoreCLR(
                    currentProcessAbsolutePath.c_str(),
                    "unixcorerun",
                    sizeof(propertyKeys) / sizeof(propertyKeys[0]),
                    propertyKeys,
                    propertyValues,
                    &hostHandle,
                    &domainId);
        if (!SUCCEEDED(st))
        {
            printf("coreclr_initialize failed - status: 0x%08x\n", st);
            exitCode = -1;
        }
        else
        {
            exitCode = 0;
        }
    }
    return exitCode;
}

void DotnetApplicationAdapter::shutdownRuntime()
{
    int st = shutdownCoreCLR(hostHandle, domainId);
    if (!SUCCEEDED(st))
    {
        printf("coreclr_shutdown failed - status: 0x%08x\n", st);
        // Error
    }
    if (dlclose(coreclrLib) != 0)
    {
        printf("Warning - dlclose failed\n");
    }
}

int DotnetApplicationAdapter::executeManagedAssemblyMain(int managedAssemblyArgc, const char** managedAssemblyArgv)
{
    // Indicates failure
    int exitCode = -1;
    int st = executeAssembly(
            hostHandle,
            domainId,
            managedAssemblyArgc,
            managedAssemblyArgv,
            managedAssemblyAbsolutePath.c_str(),
            (unsigned int*)&exitCode);

    if (!SUCCEEDED(st))
    {
        printf("coreclr_execute_assembly failed - status: 0x%08x\n", st);
        exitCode = -1;
    }

    return exitCode;
}

bool DotnetApplicationAdapter::getCoreClrHostInterface(const char* coreClrDll)
{
    coreclrLib = dlopen(coreClrDll, RTLD_NOW | RTLD_LOCAL);
    if (coreclrLib != NULL)
    {
        initializeCoreCLR = (coreclr_initialize_ptr)dlsym(coreclrLib, "coreclr_initialize");
        executeAssembly = (coreclr_execute_assembly_ptr)dlsym(coreclrLib, "coreclr_execute_assembly");
        createDelegate = (coreclr_create_delegate_ptr)dlsym(coreclrLib, "coreclr_create_delegate");
        shutdownCoreCLR = (coreclr_shutdown_ptr)dlsym(coreclrLib, "coreclr_shutdown");

        if (initializeCoreCLR == NULL){
            printf("Function coreclr_initialize not found in the libcoreclr.so\n");
            return false;
        }
        else if (executeAssembly == NULL){
            printf("Function coreclr_execute_assembly not found in the libcoreclr.so\n");
            return false;
        }
        else if (createDelegate == NULL){
            printf("Function coreclr_crate_delegate not found in the libcoreclr.so\n");
            return false;
        }
        else if (shutdownCoreCLR == NULL){
            printf("Function coreclr_shutdown_2 not found in the libcoreclr.so\n");
            return false;
        }
    }
    else
    {
        const char* error = dlerror();
        printf("dlopen failed to open the libcoreclr.so with error %s\n", error);
        return false;
    }
    return true;
}

bool DotnetApplicationAdapter::getAbsolutePath(const char* path, std::string& absolutePath)
{
    bool result = false;

    char realPath[PATH_MAX];
    if (realpath(path, realPath) != NULL && realPath[0] != '\0')
    {
        absolutePath.assign(realPath);
        // realpath should return canonicalized path without the trailing slash
        assert(absolutePath.back() != '/');

        result = true;
    }

    return result;
}

bool DotnetApplicationAdapter::getDirectory(const char* absolutePath, std::string& directory)
{
    directory.assign(absolutePath);
    size_t lastSlash = directory.rfind('/');
    if (lastSlash != std::string::npos)
    {
        directory.erase(lastSlash);
        return true;
    }

    return false;
}

const char* DotnetApplicationAdapter::GetEnvValueBoolean(const char* envVariable)
{
    const char* envValue = std::getenv(envVariable);
    if (envValue == NULL)
    {
        envValue = "0";
    }
    // CoreCLR expects strings "true" and "false" instead of "1" and "0".
    return (std::strcmp(envValue, "1") == 0 || strcasecmp(envValue, "true") == 0) ? "true" : "false";
}

void DotnetApplicationAdapter::addFilesFromDirectoryToTpaList(const char* directory)
{
    const char * const tpaExtensions[] = {
                ".ni.dll",      // Probe for .ni.dll first so that it's preferred if ni and il coexist in the same dir
                ".dll",
                ".ni.exe",
                ".exe",
                };

    DIR* dir = opendir(directory);
    if (dir == NULL)
    {
        return;
    }

    std::set<std::string> addedAssemblies;

    // Walk the directory for each extension separately so that we first get files with .ni.dll extension,
    // then files with .dll extension, etc.
    for (int extIndex = 0; extIndex < sizeof(tpaExtensions) / sizeof(tpaExtensions[0]); extIndex++)
    {
        const char* ext = tpaExtensions[extIndex];
        int extLength = strlen(ext);

        struct dirent* entry;

        // For all entries in the directory
        while ((entry = readdir(dir)) != NULL)
        {
            // We are interested in files only
            switch (entry->d_type)
            {
            case DT_REG:
                break;

            // Handle symlinks and file systems that do not support d_type
            case DT_LNK:
            case DT_UNKNOWN:
                {
                    std::string fullFilename;

                    fullFilename.append(directory);
                    fullFilename.append("/");
                    fullFilename.append(entry->d_name);

                    struct stat sb;
                    if (stat(fullFilename.c_str(), &sb) == -1)
                    {
                        continue;
                    }

                    if (!S_ISREG(sb.st_mode))
                    {
                        continue;
                    }
                }
                break;

            default:
                continue;
            }

            std::string filename(entry->d_name);

            // Check if the extension matches the one we are looking for
            int extPos = filename.length() - extLength;
            if ((extPos <= 0) || (filename.compare(extPos, extLength, ext) != 0))
            {
                continue;
            }

            std::string filenameWithoutExt(filename.substr(0, extPos));

            // Make sure if we have an assembly with multiple extensions present,
            // we insert only one version of it.
            if (addedAssemblies.find(filenameWithoutExt) == addedAssemblies.end())
            {
                addedAssemblies.insert(filenameWithoutExt);

                tpaList.append(directory);
                tpaList.append("/");
                tpaList.append(filename);
                tpaList.append(":");
            }
        }

        // Rewind the directory stream to be able to iterate over it for the next extension
        rewinddir(dir);
    }

    closedir(dir);
}

DotnetApplicationAdapter::DotnetApplicationAdapter()
{

}

DotnetApplicationAdapter::~DotnetApplicationAdapter()
{
    cancelAndDelete(timer);
    delete factory;
}

} //namespace
