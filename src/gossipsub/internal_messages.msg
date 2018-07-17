message MulticastControl
{
    int addReceivers[];
    int removeReceivers[];
};

message GardenerControl
{
    int pruneReceivers[];
    int graftReceivers[];
};

message CacheQuery
{
    int contentId;
};

message CacheQueryResponse
{
    bool found;
};
