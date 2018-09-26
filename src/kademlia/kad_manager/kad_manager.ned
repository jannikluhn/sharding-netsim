package sharding.kademlia.kad_manager;

simple KadManager
{
    parameters:
        string peerTablePath;
        int nodeId;
        bool hidden;

        int lookupConcurrency;
        double maxLookupRoundDuration @unit(s);
        double lookupInterval @unit(s);

        int numBootnodes;

    gates:
        input neighborsInput;
        input pongInput;
        input addMeInput;
        output out;
        output peerListChangeOutput;
}
