cplusplus {{
#include "../packets_m.h"
}}
packet DiscoveryPacket;


packet MockDiscoveryPacket extends DiscoveryPacket
{
    protocol = MOCK_DISCOVERY;
}

packet ForceJoin extends MockDiscoveryPacket
{

};

