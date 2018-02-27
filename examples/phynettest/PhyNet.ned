//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//

package inet.examples.phynettest;


import inet.node.wsn.AccessPoint;
import inet.node.wsn.PhyNode;
import inet.physicallayer.idealradio.IdealRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.mobility.group.StorageHallCoordinator;

network PhyNet
{
    parameters:
        @display("bgu=m");
		int numNodes;
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=-0.3,1");
        }
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=-0.3,0.5");
        }
        mobilityCoordinator: StorageHallCoordinator {
        	parameters:
        	    @display("p=-0.3,1.5;is=s");
        	    numItems = numNodes;          
        }
        accessPoint: AccessPoint {
            parameters:
                @display("p=1,1;i=,black,40");
                //mobilityType = "StorageHallTractorMobility";
                mobilityType = "StorageHallRandomMobility";
                mobility.initAtCenter = true;
                mobility.speed = 0mps; // stationary
        }
        node[numNodes]: PhyNode {
            parameters:
                @display("i=-;b=0.3,0.4,rect,,black,1");
                mobilityType = "StorageHallGridMobility";
                mobility.speed = 0mps; // stationary
        }
}

