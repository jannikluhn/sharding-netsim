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
    int contentId;
    bool found;
};
