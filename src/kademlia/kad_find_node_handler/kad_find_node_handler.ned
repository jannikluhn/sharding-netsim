package sharding.kademlia.kad_find_node_handler;

simple KadFindNodeHandler
{
    parameters:
        string peerTablePath;
        volatile int shardId;

    gates:
        input findNodeInput;
        output out;
}
