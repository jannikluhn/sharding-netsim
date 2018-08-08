package sharding.utils.node_manager;

simple NodeManager
{
    parameters:
        string nodeTypeName;
        string hubPath;

        int bootstrapNodeCount;
        int targetNodeCount;
        double rampUpTime @unit(s);
}
