cplusplus {{
#include "../packets_m.h"
}}
packet GossipPacket;


packet EpiSubPacket extends GossipPacket
{
    protocol = EPISUB;
}


enum EpiSubPacketType {
    // GOSSIP = 0  (see shared gossip packets)
    IHAVE = 1;
    GRAFT = 2;
    PRUNE = 3;
};

packet IHave2 extends EpiSubPacket
{
    packetType = IHAVE;

    int contentIds[];
};

packet Graft2 extends EpiSubPacket
{
    packetType = GRAFT;

    int contentIds[];
};

packet Prune2 extends EpiSubPacket
{
    packetType = PRUNE;
};
