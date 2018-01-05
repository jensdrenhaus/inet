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

void ApplicationAdapter::initialize()
{
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

void ApplicationAdapter::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

} //namespace
