package sharding.gossipsub;

network TestNetwork
{
    submodules:
        nodes[3]: Node;

    connections:
        nodes[0].ports++ <--> nodes[1].ports++;
        nodes[0].ports++ <--> nodes[2].ports++;
        nodes[1].ports++ <--> nodes[2].ports++;
}

module Node
{
    gates:
        inout ports[];

    submodules:

        outgoingDispatcher: ReceiverDispatcher;
        incomingDispatcher: TypeDispatcher;

        gossipReceiver: GossipReceiver;
        iHaveReceiver: IHaveReceiver;
        gardener: Gardener;

        source: Source;
        cache: Cache;
        lazyNotifier: LazyNotifier;

        eagerMulticaster: Multicaster;
        lazyMulticaster: Multicaster;


    connections:
        for i=0..sizeof(ports) - 1 {
            ports$o[i] <-- outgoingDispatcher.outputs++;
            ports$i[i] --> incomingDispatcher.inputs++;
        }

        gossipReceiver.in <-- incomingDispatcher.gossipOutputs++;
        gossipReceiver.cacheQuery <--> cache.query++;
        gossipReceiver.cacheAdd <--> cache.add++;
        gossipReceiver.gardenerControl --> gardener.controlInputs++;
        gossipReceiver.lazyNotifierOut --> lazyNotifier.in++;

        gardener.graftIn <-- incomingDispatcher.graftOutputs++;
        gardener.pruneIn <-- incomingDispatcher.pruneOutputs++;
        gardener.eagerControl --> eagerMulticaster.control++;
        gardener.lazyControl --> lazyMulticaster.control++;

        lazyNotifier.inputs++ <-- incomingDispatcher.iHaveOutputs++;
        lazyNotifier.out --> lazyMulticaster.in;

        source.outputs++ --> eagerMulticaster.inputs++;
        source.outputs++ --> lazyNotifier.inputs++;
        source.cacheAdd --> cache.add++;

        eagerMulticaster.out --> outgoingDispatcher.inputs++;
        lazyMulticaster.out --> outgoingDispatcher.inputs++;
}

simple Source
{
    gates:
        output out;
}

simple Funnel
{
    gates:
        input[] inputs;
        output[] outputs;
}

simple ReceiverDispatcher
{
    gates:
        input[] inputs;
        output[] outputs;
}

simple TypeDispatcher
{
    gates:
        input: in;
        output: gossipOut;
        output: iHaveOut;
        output: pruneOut;
        output: graftOut;
}

simple Cache
{
    gates:
        input[]: add;
        inout[]: cacheQuery;
}

simple GossipReceiver
{
    gates:
        input: in;
        output: gardenerControl;
        inout: cacheQuery;
        output: cacheAdd;
        output: lazyNotifierOut;
}

simple Gardener
{
    gates:
        input pruneIn;
        input graftIn;
        output out;
        output eagerControl;
        output lazyControl;
        input[] controlInputs;
}

simple Multicaster
{
    gates:
        input[] inputs;
        output[] outputs;
        input[] control;
}
