package sharding.gossipsub.gardener;


// Module that prunes and grafts connections.
simple Gardener
{
    gates:
        input graftInput;
        input pruneInput;
        inout cacheQueryPort;
        output graftResponseOutput;
        output eagerMulticastControlOutput;
        output lazyMulticastControlOutput;
        input controlInput;
}
