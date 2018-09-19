package sharding.kademlia.kad_neighbors_handler;

simple KadNeighborsHandler
{
    parameters:
        string peerTablePath;
        volatile int shardId;

    gates:
        input neighborsInput;
        output out;
}
