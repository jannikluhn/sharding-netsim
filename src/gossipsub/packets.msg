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

packet Shuffle extends AddressedPacket
{
    int node;
    int peers[];
    int ttl;
}

packet ShuffleReply extends AddressedPacket
{
    int peers[];
}

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

