package sharding.gossipsub.connection_manager;


simple ConnectionManager
{
    parameters:
        int minPeers;
        int maxPeers;

    gates:
        output out;
        input joinInput;
        input joinResponseInput;
        output newPeerOutputs[];
}
