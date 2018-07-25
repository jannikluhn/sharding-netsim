package sharding.gossipsub.connection_manager;


simple ConnectionManager
{
    parameters:
        int minPeers;
        int maxPeers;

    gates:
        output out;

        input getNodesInput;
        input nodesInput;
        input joinInput;
        input joinResponseInput;

        output newPeerOutputs[];
}
