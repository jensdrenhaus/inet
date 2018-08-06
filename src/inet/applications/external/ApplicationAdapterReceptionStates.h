//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe
//

#ifndef __INET_APPLICATIONS_EXTERNAL_APPLICATIONADAPTERRECEPTIOSTATES_H
#define __INET_APPLICATIONS_EXTERNAL_APPLICATIONADAPTERRECEPTIOSTATES_H

namespace inet {

/**
 * Initialization stages.
 */
enum ReceptionStates {
    /**
     * Message is received correctly
     */
    OK = 0,

    /**
     * Message is ignored by the receiver because of other ongoing receptions
     */
    IGNORED = 1,

    /**
     * Message
     */
    BITERROR = 2,


};

} // namespace inet

#endif    //


