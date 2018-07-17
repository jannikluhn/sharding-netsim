packet AddressedPacket
{
    int sender;
    int receiver;
};

packet Gossip extends AddressedPacket
{
    int contentIds[];
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
