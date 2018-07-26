package sharding.gossipsub.membership.passive_list_manager;

simple PassiveListManager
{
    parameters:
        string peerListPath;

    gates:
        input nodesInput;
        output viewInitializationCompleteOutputs[];
        output out;
};
