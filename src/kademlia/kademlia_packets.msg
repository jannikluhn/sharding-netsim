cplusplus {{
#include "../packets_m.h"
}}
packet DiscoveryPacket;


packet KademliaPacket extends DiscoveryPacket
{
    protocol = KADEMLIA;

    int shard = -1;
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

    int targetNode;
    int targetShard;
};

packet KadNeighbors extends KademliaPacket
{
    packetType = NEIGHBORS;

    int nodes[];
    int shards[];
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
