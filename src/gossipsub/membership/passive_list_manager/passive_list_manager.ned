package sharding.gossipsub.membership.passive_list_manager;

simple PassiveListManager
{
    parameters:
        string peerListPath;
        int nodeId;

        int shuffleInterval;
        int activeShufflingSize;  // k_a
        int passiveShufflingSize;  // k_p
        int shuffleTTL;

    gates:
        input nodesInput;
        input shuffleInput;
        input shuffleReplyInput;
        output viewInitializationFinishedOutputs[];
        output out;
};
