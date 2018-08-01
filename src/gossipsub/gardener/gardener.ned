package sharding.gossipsub.gardener;


// Module that prunes and grafts connections.
simple Gardener
{
    parameters:
        string cachePath;

        @signal[newGossipReceived](type=long);

    gates:
        input graftInput;
        input pruneInput;
        input gossipInput;
        input eagerMulticastInputs[];
        input lazyMulticastInputs[];
        output out;

        input addedActivePeerInput;
        input removedActivePeerInput;
}
