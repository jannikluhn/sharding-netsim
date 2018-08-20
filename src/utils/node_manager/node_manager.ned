package sharding.utils.node_manager;

simple NodeManager
{
    parameters:
        string hubPath;

        int bootstrapNodeCount;
        int targetNodeCount;
        double rampUpTime @unit(s);
}
