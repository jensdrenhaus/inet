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

package inet.applications.external;


//
// TODO auto-generated module
//
simple DotnetApplicationAdapter extends ApplicationAdapterBase
{
    parameters:
        @class(DotnetApplicationAdapter);
        @display("i=block/filter_s");
        
        
        string assembly = default("SampleApp.dll"); // path to assambly relative to src/csharp/
        string namespaceName = default("OmnetServices"); // namespace used in the assembly 
        string className = default("OmnetInterface"); // class inside the assembly that implements the callback methods
        string clrFilesPath = default("/usr/share/dotnet/shared/Microsoft.NETCore.App/2.1.2"); // absolut path to the dotnet installations' CLR files
        
        string nodeType = default("inet.node.wsn.TrampolineNode"); // fully qualified NED type name of the module which is to be dynamically instanciated
        
    gates:
        input appsIn @directIn;
}