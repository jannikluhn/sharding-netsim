cplusplus {{
#include "../packets_m.h"
}}
packet DiscoveryPacket;


packet KademliaPacket extends DiscoveryPacket
{
    protocol = KADEMLIA;
}


enum KademliaPacketType {
    FIND_NODE = 0;
    NEIGHBORS = 1;
    PING = 2;
    PONG = 3;
    ADD_ME = 4;
};

packet KadPing extends KademliaPacket
{
    packetType = PING;
};

packet KadPong extends KademliaPacket
{
    packetType = PONG;
};
