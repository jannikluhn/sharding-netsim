cplusplus {{
#include "../packets_m.h"
#include "kademlia/kad_id.h"
}}

packet DiscoveryPacket;
class noncobject KadId;


packet KademliaPacket extends DiscoveryPacket
{
    protocol = KADEMLIA;

    KadId senderKadId;
};


enum KademliaPacketType {
    FIND_NODE = 0;
    NEIGHBORS = 1;
    PING = 2;
    PONG = 3;
    ADD_ME = 4;
};

packet KadFindNode extends KademliaPacket
{
    packetType = FIND_NODE;

    KadId targetKadId;
};

packet KadNeighbors extends KademliaPacket
{
    packetType = NEIGHBORS;

    KadId kadIds[];
    int nodeIds[];
};

packet KadPing extends KademliaPacket
{
    packetType = PING;
};

packet KadPong extends KademliaPacket
{
    packetType = PONG;
};

packet KadAddMe extends KademliaPacket
{
    packetType = ADD_ME;
};
