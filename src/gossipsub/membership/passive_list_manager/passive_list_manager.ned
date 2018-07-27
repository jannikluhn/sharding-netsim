package sharding.gossipsub.membership.passive_list_manager;

simple PassiveListManager
{
    parameters:
        string peerListPath;
        int nodeId;

    gates:
        input nodesInput;
        output viewInitializationFinishedOutputs[];
        output out;
};
