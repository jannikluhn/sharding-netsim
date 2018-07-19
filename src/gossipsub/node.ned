package sharding.gossipsub;

import sharding.gossipsub.receiver_dispatcher.ReceiverDispatcher;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;
import sharding.gossipsub.source.Source;
import sharding.gossipsub.cache.Cache;
import sharding.gossipsub.multicaster.Multicaster;
import sharding.gossipsub.gardener.Gardener;
import sharding.gossipsub.lazy_notifier.LazyNotifier;


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

        source: Source;
        cache: Cache;
        gardener: Gardener;
        lazyNotifier: LazyNotifier;
//      missingTracker: MissingTracker;

        eagerMulticaster: Multicaster;
        lazyMulticaster: Multicaster;

    connections allowunconnected:
        for i=0..sizeof(ports) - 1 {
            ports$o[i] <-- outgoingDispatcher.outputs++;
            ports$i[i] --> incomingDispatcher.inputs++;
        }

        cache.addGossipInputs++ <-- source.out;
        cache.addGossipInputs++ <-- incomingDispatcher.gossipOutputs++;
        cache.newGossipOutputs++ --> eagerMulticaster.inputs++;
        cache.gardenerControlOutput --> gardener.controlInput;

        gardener.graftInput <-- incomingDispatcher.graftOutputs++;
        gardener.pruneInput <-- incomingDispatcher.pruneOutputs++;
        gardener.cacheQueryPort <--> cache.queryPorts++;
        gardener.graftResponseOutput --> outgoingDispatcher.inputs++;
        gardener.eagerMulticastControlOutput --> eagerMulticaster.controlInput;
        gardener.lazyMulticastControlOutput --> lazyMulticaster.controlInput;

        lazyNotifier.gossipInput <-- cache.newGossipOutputs++;
        lazyNotifier.notificationOutput --> lazyMulticaster.inputs++;

//      missingTracker.iHaveInput <-- incomingDispatcher.iHaveOutputs++;
//      missingTracker.cacheQueryPort <--> cache.queryPorts++;
//      missingTracker.out --> outgoingDispatcher.inputs++;

        eagerMulticaster.out --> outgoingDispatcher.inputs++;
        lazyMulticaster.out --> outgoingDispatcher.inputs++;
}
