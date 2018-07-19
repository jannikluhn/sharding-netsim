package sharding.gossipsub;

import sharding.gossipsub.receiver_dispatcher.ReceiverDispatcher;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;
import sharding.gossipsub.source.Source;
import sharding.gossipsub.cache.Cache;
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

    connections allowunconnected:
        for i=0..sizeof(ports) - 1 {
            ports$o[i] <-- outgoingDispatcher.outputs++;
            ports$i[i] --> incomingDispatcher.inputs++;
        }

        cache.addGossipInputs++ <-- source.out;
        cache.addGossipInputs++ <-- incomingDispatcher.gossipOutputs++;
        cache.newGossipOutputs++ --> gardener.eagerMulticastInputs++;
        cache.gardenerControlOutput --> gardener.controlInput;

        gardener.graftInput <-- incomingDispatcher.graftOutputs++;
        gardener.pruneInput <-- incomingDispatcher.pruneOutputs++;
        gardener.cacheQueryPort <--> cache.queryPorts++;
        gardener.out --> outgoingDispatcher.inputs++;

        lazyNotifier.gossipInput <-- cache.newGossipOutputs++;
        lazyNotifier.notificationOutput --> gardener.lazyMulticastInputs++;

//      missingTracker.iHaveInput <-- incomingDispatcher.iHaveOutputs++;
//      missingTracker.cacheQueryPort <--> cache.queryPorts++;
//      missingTracker.out --> outgoingDispatcher.inputs++;
}
