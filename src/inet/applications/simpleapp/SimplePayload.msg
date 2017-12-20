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

cplusplus {{
#include "inet/common/INETDefs.h"
}}

namespace inet;

//
// Represents payload in an MAC Echo Request/Reply.
//
// TODO: move originator id and sequence number to ICMPMessage and use an ICMPControlInfo
packet SimplePayload
{
    long originatorId; // sender module id
    long seqNo; // sequence number
    unsigned char data[];
}