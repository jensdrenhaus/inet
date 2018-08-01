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

package inet.examples.phynettest_dotnet_csharp_demo;

import inet.node.wsn.TrampolineNode;
import inet.physicallayer.idealradio.IdealRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.applications.external.DotnetApplicationAdapter;


network Net
{
    parameters:
        //int numNodes;
        @display("bgb=600,400");
    submodules:
//        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
//            parameters:
//                @display("p=-100,50");
//        }
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=-100,250");
        }
        adapter: DotnetApplicationAdapter {
        	parameters:
        	    @display("p=-100,150");
        	    assembly     = "csharp_demo/bin/Debug/netcoreapp2.0/publish/SampleApp.dll"; // path to assambly relative to src/csharp/
        	    clrFilesPath = "/usr/share/dotnet/shared/Microsoft.NETCore.App/2.1.2"; // absolut path to the dotnet CLR files
        }
//        node[numNodes]: PhyNode {
//            parameters:
//                @display("r=,,#707070;p=250,58");
//        }
        node: TrampolineNode {
            parameters:
                @dynamic;
        }
}