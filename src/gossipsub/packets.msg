packet AddressedPacket
{
    // default to -1 to not clash with any module ids by accident
    int sender = -1;
    int receiver = -1;
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

packet Join extends AddressedPacket
{

};

packet JoinResponse extends AddressedPacket
{
    bool success;
};
