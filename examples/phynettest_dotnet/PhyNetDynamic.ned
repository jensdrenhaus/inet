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

package inet.examples.phynettest_dotnet;

import inet.node.wsn.PhyNodeExternal;
import inet.physicallayer.idealradio.IdealRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.applications.external.ApplicationAdapter;


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
        adapter: ApplicationAdapter {
        	parameters:
        	    @display("p=-100,150");
        }
//        node[numNodes]: PhyNode {
//            parameters:
//                @display("r=,,#707070;p=250,58");
//        }
        node: PhyNodeExternal {
            parameters:
                @dynamic;
        }
}