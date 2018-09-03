cplusplus {{
#include "../packets_m.h"
}}
packet DiscoveryPacket;


packet MockDiscoveryPacket extends DiscoveryPacket
{
    protocol = MOCK_DISCOVERY;
}

enum MockDiscoveryPacketType {
    FORCE_JOIN = 0;
};

packet ForceJoin extends MockDiscoveryPacket
{
    packetType = FORCE_JOIN;

    int64_t bitLength = 0;
};
