//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211PHYHEADERSERIALIZER_H
#define __INET_IEEE80211PHYHEADERSERIALIZER_H

#include "inet/common/packet/Serializer.h"

namespace inet {

namespace serializer {

/**
 * Converts between Ieee80211PhyHeader and binary network byte order IEEE 802.11 phy header.
 */
class INET_API Ieee80211PhyHeaderSerializer
{
  protected:
//    void writeToBitVector(unsigned char *buf, unsigned int bufSize, BitVector *bitVector) const;
//    bool serialize(const inet::physicallayer::Ieee80211PLCPFrame *plcpHeader, BitVector *serializedPacket) const;
//    inet::physicallayer::Ieee80211PLCPFrame *deserialize(BitVector *serializedPacket) const;

  public:
    Ieee80211PhyHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_IEEE80211PHYHEADERSERIALIZER_H