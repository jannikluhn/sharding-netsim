packet AddressedPackage
{
    int sender;
    int receiver;
};

packet Gossip extends AddressedPackage
{
    int contentId;
};

packet IHave extends AddressedPackage
{
    int contentIds[];
};

packet Graft extends AddressedPackage
{
    int contentIds[];
};

packet Prune extends AddressedPackage
{

};

// internal messages
message MulticastControl
{
    int[] addedReceivers;
    int[] removedReceivers;
};

message GardenerControl
{
    int[] prunedModules;
    int[] graftedModules;
};

message CacheAdd
{
    int contentId;
};

message CacheQuery
{
    int contentId;
};

message CacheQueryResponse
{
    bool found;
};
