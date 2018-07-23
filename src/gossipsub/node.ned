package sharding.gossipsub;

import sharding.gossipsub.hub.Hub;
import sharding.gossipsub.funnel.Funnel;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;
import sharding.gossipsub.source.Source;
import sharding.gossipsub.cache.Cache;
import sharding.gossipsub.gardener.Gardener;
import sharding.gossipsub.lazy_notifier.LazyNotifier;
import sharding.gossipsub.missing_tracker.MissingTracker;
import sharding.gossipsub.connection_manager.ConnectionManager;


network Network
{
    parameters:
        int nodeCount;
        double gossipRate @unit(Hz);

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count,mean,histogram);
        @statistic[messageSent](record=count);

    submodules:
        nodes[nodeCount]: Node {
            source.rate = gossipRate / nodeCount;
        }
        hub: Hub;

    connections:
        for i=0..nodeCount-1 {
            nodes[i].port <--> hub.ports++;
        }
}


module Node
{
    gates:
        inout port;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        source: Source;
        cache: Cache;
        gardener: Gardener;
        lazyNotifier: LazyNotifier;
        missingTracker: MissingTracker;
        connectionManager: ConnectionManager;

    connections:
        port$i --> inputDispatcher.in;
        port$o <-- outputFunnel.out;

        cache.addGossipInputs++ <-- source.out;
        cache.addGossipInputs++ <-- inputDispatcher.gossipOutputs++;
        cache.newGossipOutputs++ --> gardener.eagerMulticastInputs++;
        cache.gardenerControlOutput --> gardener.controlInput;

        gardener.graftInput <-- inputDispatcher.graftOutputs++;
        gardener.pruneInput <-- inputDispatcher.pruneOutputs++;
        gardener.cacheQueryPort <--> cache.queryPorts++;
        gardener.newPeerInput <-- connectionManager.newPeerOutputs++;
        gardener.out --> outputFunnel.inputs++;

        lazyNotifier.gossipInput <-- cache.newGossipOutputs++;
        lazyNotifier.notificationOutput --> gardener.lazyMulticastInputs++;

        missingTracker.iHaveInput <-- inputDispatcher.iHaveOutputs++;
        missingTracker.cacheQueryPort <--> cache.queryPorts++;
        missingTracker.out --> outputFunnel.inputs++;

        connectionManager.joinInput <-- inputDispatcher.joinOutputs++;
        connectionManager.joinResponseInput <-- inputDispatcher.joinResponseOutputs++;
        connectionManager.out --> outputFunnel.inputs++;
}
