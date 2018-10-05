package sharding.utils.node_manager;

simple NodeManager
{
    parameters:
        string hubPath;
        string sourcePath;

        int bootstrapNodeCount;
        int nodeCount;

        double meanLifetime @unit(s);
}
