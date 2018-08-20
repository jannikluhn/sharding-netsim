cplusplus {{
#include "../packets_m.h"
}}
packet DiscoveryPacket;


packet HyParViewPacket extends DiscoveryPacket
{
    protocol = HYPARVIEW;
}


enum HyParViewPacketType {
    GETNODES = 0;
    NODES = 1;
    JOIN = 2;
    FORWARDJOIN = 3;
    NEIGHBOR = 4;
    DISCONNECT = 5;
    SHUFFLE = 6;
    SHUFFLEREPLY = 7;
};

packet GetNodes extends HyParViewPacket
{
    packetType = GETNODES;
};

packet Nodes extends HyParViewPacket
{
    packetType = NODES;

    int peers[];
};

packet Join extends HyParViewPacket
{
    packetType = JOIN;

    int node;
    int ttl;
};

packet ForwardJoin extends HyParViewPacket
{
    packetType = FORWARDJOIN;

    int node;
    int ttl;
};

packet Neighbor extends HyParViewPacket
{
    packetType = NEIGHBOR;
};

packet Disconnect extends HyParViewPacket
{
    packetType = DISCONNECT;
};

packet Shuffle extends HyParViewPacket
{
    packetType = SHUFFLE;

    int node;
    int peers[];
    int ttl;
};

packet ShuffleReply extends HyParViewPacket
{
    packetType = SHUFFLEREPLY;

    int peers[];
};
