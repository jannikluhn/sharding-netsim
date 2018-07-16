packet Gossip
{
    int contentId;
};

packet IHave
{
    int contentIds[];
};

packet Graft
{
    int contentIds[];
};

packet Prune
{

};
