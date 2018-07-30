package sharding.gossipsub.membership.joiner;


simple Joiner
{
    parameters:
        string peerListPath;
        int nodeId;
        int numRandomNeighbors;  // c_rand
        int numNearNeighbors;  // c_near
        double heartbeatInterval;
        int joinTTL;
        int forwardJoinTTL;

    gates:
        input joinInput;
        input neighborInput;
        input forwardJoinInput;
        input disconnectInput;
        input startInput;
        output out;
}
