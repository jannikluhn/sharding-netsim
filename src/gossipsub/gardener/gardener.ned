package sharding.gossipsub.gardener;


// Module that prunes and grafts connections.
simple Gardener
{
    gates:
        input graftInput;
        input pruneInput;
        input eagerMulticastInputs[];
        input lazyMulticastInputs[];
        inout cacheQueryPort;
        input newPeerInput;
        output out;
        input controlInput;
}
