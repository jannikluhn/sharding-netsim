packet AddressedPacket
{
    // default to -1 to not clash with any module ids by accident
    int sender = -1;
    int receiver = -1;
};

packet GetNodes extends AddressedPacket
{

};

packet Nodes extends AddressedPacket
{
    int peers[];
};

packet Join extends AddressedPacket
{
    int peer;
    int ttl;
};

packet ForwardJoin extends AddressedPacket
{
    int peer;
    int ttl;
};

packet Neighbor extends AddressedPacket
{
    int peer;
};

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

