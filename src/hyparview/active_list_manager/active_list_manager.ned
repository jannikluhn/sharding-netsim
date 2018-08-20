package sharding.hyparview.active_list_manager;


simple ActiveListManager
{
    parameters:
        string peerListPath;
        int nodeId;
        int numRandomNeighbors;  // c_rand
        int numNearNeighbors;  // c_near
        double heartbeatInterval @unit(s);
        int joinTTL;
        int forwardJoinTTL;

        @signal[peerListUpdate](type=long);
        @signal[activeListUpdate](type=long);

    gates:
        input joinInput;
        input neighborInput;
        input disconnectInput;
        input startInput;
        output out;
        output activeListChangeOutput;
}
