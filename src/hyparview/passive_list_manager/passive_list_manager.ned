package sharding.hyparview.passive_list_manager;

simple PassiveListManager
{
    parameters:
        string peerListPath;
        int nodeId;

        double shuffleInterval;
        int activeShufflingSize;  // k_a
        int passiveShufflingSize;  // k_p
        int shuffleTTL;
        int passiveListSize;

        @signal[passiveListUpdate](type=long);

    gates:
        input nodesInput;
        input shuffleInput;
        input shuffleReplyInput;
        input forwardJoinInput;
        output viewInitializationFinishedOutputs[];
        output out;
};
