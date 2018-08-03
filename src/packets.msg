packet AddressedPacket
{
    // default to -1 to not clash with any module ids by accident
    int sender = -1;
    int receiver = -1;
};


//
// HyParView
//
packet GetNodes extends AddressedPacket
{

};

packet Nodes extends AddressedPacket
{
    int peers[];
};

packet Join extends AddressedPacket
{
    int node;
    int ttl;
};

packet Neighbor extends AddressedPacket
{

};

packet Disconnect extends AddressedPacket
{

};

packet ForwardJoin extends AddressedPacket
{
    int node;
    int ttl;
};

packet Shuffle extends AddressedPacket
{
    int node;
    int peers[];
    int ttl;
};

packet ShuffleReply extends AddressedPacket
{
    int peers[];
};


//
// Broadcast Tree Gossipsub
//
packet Gossip extends AddressedPacket
{
    int contentIds[];
    int hops;
};

packet IHave extends AddressedPacket
{
    int contentIds[];
};

packet Graft extends AddressedPacket
{
    int contentIds[];
};

packet Prune extends AddressedPacket
{

};
