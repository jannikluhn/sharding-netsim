cplusplus {{
#include "../packets_m.h"
}}
packet GossipPacket;


packet PushPullPacket extends GossipPacket
{
    protocol = PUSHPULL;
}


enum PushPullPacketType {
    // GOSSIP = 0  (see shared gossip packets)
    PULL = 1;
};


packet Pull extends PushPullPacket {
    packetType = PULL;

    int contentIds[];

    int64_t bitLength = (100 + 32) * 8;
};
