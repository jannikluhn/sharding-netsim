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

    int64_t bitLength = (100 + 32) * 8;
};

packet Graft2 extends EpiSubPacket
{
    packetType = GRAFT;

    int contentIds[];

    int64_t bitLength = (100 + 32) * 8;
};

packet Prune2 extends EpiSubPacket
{
    packetType = PRUNE;

    int64_t bitLength = 100 * 8;
};
