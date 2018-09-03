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

    int64_t bitLength = 100 + 32 * 3;
};

packet Join extends HyParViewPacket
{
    packetType = JOIN;

    int node;
    int ttl;

    int64_t bitLength = 100 + 32 + 1;
};

packet ForwardJoin extends HyParViewPacket
{
    packetType = FORWARDJOIN;

    int node;
    int ttl;

    int64_t bitLength = 100 + 32 + 1;
};

packet Neighbor extends HyParViewPacket
{
    packetType = NEIGHBOR;

    int64_t bitLength = 100;
};

packet Disconnect extends HyParViewPacket
{
    packetType = DISCONNECT;

    int64_t bitLength = 100;
};

packet Shuffle extends HyParViewPacket
{
    packetType = SHUFFLE;

    int node;
    int peers[];
    int ttl;

    int64_t bitLength = 100 + 32 + 1 + 32 * 3;
};

packet ShuffleReply extends HyParViewPacket
{
    packetType = SHUFFLEREPLY;

    int peers[];

    int64_t bitLength = 100 + 32 * 3;
};
