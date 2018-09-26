package sharding.kademlia.kad_find_node_handler;

simple KadFindNodeHandler
{
    parameters:
        string peerTablePath;

    gates:
        input findNodeInput;
        output out;
}
