cplusplus {{
#include "../packets_m.h"
}}
packet GossipPacket;


packet GossipSubPacket extends GossipPacket
{
    protocol = GOSSIPSUB;
}


enum GossipSubPacketType {
    // GOSSIP = 0  (see shared gossip packets)
    IHAVE = 1;
    IWANT = 2;
    GRAFT = 3;
    PRUNE = 4;
};


packet IHave extends GossipSubPacket {
    packetType = IHAVE;

    int contentIds[];
};

packet IWant extends GossipSubPacket {
    packetType = IWANT;

    int contentIds[];
};

packet Graft extends GossipSubPacket {
    packetType = GRAFT;
};

packet Prune extends GossipSubPacket {
    packetType = PRUNE;
};
