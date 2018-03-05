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

package inet.node.wsn;

import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IWirelessNic;
import inet.node.wsn.ITinyNode;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.power.contract.IEnergyGenerator;
import inet.power.contract.IEnergyManagement;
import inet.power.contract.IEnergyStorage;

// NOTE:
//    - use this model only as a base type. In simulations use 
//      TODO
//
module TinyNodeBase like ITinyNode
{
    parameters:
        @display("bgb=,448");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        int numRadios = 1;
        string osgModel = default(""); // 3D model for OSG visualization, no 3D model by default
        string osgModelColor = default(""); // tint color, no colorization by default
        string mobilityType = default("StationaryMobility");
        string energyStorageType = default("");
        string energyManagementType = default("");
        string energyGeneratorType = default("");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");
	gates:
	    input radioIn @directIn;
	submodules:
	    energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=50,100;i=block/plug;is=s");
        }
        energyManagement: <energyManagementType> like IEnergyManagement if energyManagementType != "" {
            parameters:
                @display("p=50,125;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=50,150;i=block/plug;is=s");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=53,200");
        }
        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,300;is=s");
        }
        nic: <default("SimpleWirelessNic")> like IWirelessNic {
            parameters:
                @display("p=216,241,;q=queue");
        }
	connections allowunconnected:
        radioIn --> { @display("m=s"); } --> nic.radioIn;
}