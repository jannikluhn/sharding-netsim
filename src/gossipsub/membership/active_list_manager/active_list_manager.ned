package sharding.gossipsub.membership.active_list_manager;


simple ActiveListManager
{
    parameters:
        string peerListPath;
        int nodeId;
        int numRandomNeighbors;  // c_rand
        int numNearNeighbors;  // c_near
        double heartbeatInterval;
        int joinTTL;
        int forwardJoinTTL;

        @signal[activeListUpdate](type=long);

    gates:
        input joinInput;
        input neighborInput;
        input forwardJoinInput;
        input disconnectInput;
        input startInput;
        output out;
}
