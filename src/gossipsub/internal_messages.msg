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

message ActiveListChange
{
    int added;
    int removed;
};

message ViewInitializationComplete
{

};
