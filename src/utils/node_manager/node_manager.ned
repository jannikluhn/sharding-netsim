package sharding.utils.node_manager;

simple NodeManager
{
    parameters:
        string hubPath;
        string sourcePath;

        int bootstrapNodeCount;
        int targetNodeCount;
        double rampUpTime @unit(s);

        bool crash = default(false);
        double crashTime @unit(s) = default(0);
        double crashProbability = default(0);
}
