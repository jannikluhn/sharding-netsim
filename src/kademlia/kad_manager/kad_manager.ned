package sharding.kademlia.kad_manager;

simple KadManager
{
    parameters:
        string peerTablePath;
        int nodeId;
        int shardId;
        bool hidden;

        int lookupConcurrency;
        double maxLookupRoundDuration @unit(s);

        int maxPeers;
        int numBootnodes;

    gates:
        input neighborsInput;
        input pongInput;
        output out;
        output peerListChangeOutput;
}
