//
// Internal messages
//
message PeerListChange
{
    int addedPeers[];
    int removedPeers[];
}


//
// Packets
//
enum ProtocolClass
{
    DISCOVERY_PROTOCOL = 0;
    GOSSIP_PROTOCOL = 1;
};

enum DiscoveryProtocol
{
    MOCK_DISCOVERY = 0;
    HYPARVIEW = 1;
};

enum GossipProtocol
{
    SHARED = 0;
    MOCK_GOSSIP = 1;
    FLOODSUB = 2;
    GOSSIPSUB = 3;
    EPISUB = 4;
    PUSHPULL = 5;
}

packet AddressedPacket
{
    // default to -1 to not clash with any module ids by accident
    int sender = -1;
    int receiver = -1;

    int protocolClass @enum(ProtocolClass);
    int protocol = -1;
    int packetType = -1;
};


packet DiscoveryPacket extends AddressedPacket
{
    protocolClass = DISCOVERY_PROTOCOL;
    protocol @enum(DiscoveryProtocol) = -1;
};

packet GossipPacket extends AddressedPacket
{
    protocolClass = GOSSIP_PROTOCOL;
    protocol @enum(GossipProtocol) = -1;
};


//
// Packets shared between all gossip protocols
//
packet SharedGossipPacket extends GossipPacket {
    protocol = SHARED;
};

enum SharedPacketType {
    GOSSIP = 0;
};

packet Gossip extends SharedGossipPacket
{
    packetType = GOSSIP;

    int contentId;
    int hops;
    simtime_t creationTime;

    int64_t bitLength = 1 * 1024 * 1024 * 8;
};
