package sharding.gossipsub;

import sharding.gossipsub.hub.Hub;
import sharding.gossipsub.funnel.Funnel;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;
import sharding.gossipsub.source.Source;
import sharding.gossipsub.cache.Cache;
import sharding.gossipsub.gardener.Gardener;
import sharding.gossipsub.lazy_notifier.LazyNotifier;
import sharding.gossipsub.missing_tracker.MissingTracker;
import sharding.gossipsub.membership.ConnectionManager;


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
            nodeId = index;
            source.rate = gossipRate / nodeCount;
        }
        hub: Hub;

    connections:
        for i=0..sizeof(nodes)-1 {
            nodes[i].port <--> { delay=100ms; } <--> hub.ports++;
        }
}


module Node
{
    parameters:
        int nodeId;

    gates:
        inout port;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        source: Source {
            nodeId = nodeId;
        };
        cache: Cache {
            nodeId = nodeId;
        }
        gardener: Gardener;
        lazyNotifier: LazyNotifier;
        missingTracker: MissingTracker;

        connectionManager: ConnectionManager {
            nodeId = nodeId;
        };

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
        gardener.addedActivePeerInput <-- connectionManager.addedActivePeerOutput;
        gardener.removedActivePeerInput <-- connectionManager.removedActivePeerOutput;
        gardener.out --> outputFunnel.inputs++;

        lazyNotifier.gossipInput <-- cache.newGossipOutputs++;
        lazyNotifier.notificationOutput --> gardener.lazyMulticastInputs++;

        missingTracker.iHaveInput <-- inputDispatcher.iHaveOutputs++;
        missingTracker.cacheQueryPort <--> cache.queryPorts++;
        missingTracker.out --> outputFunnel.inputs++;

        connectionManager.getNodesInput <-- inputDispatcher.getNodesOutputs++;
        connectionManager.nodesInput <-- inputDispatcher.nodesOutputs++;
        connectionManager.joinInput <-- inputDispatcher.joinOutputs++;
        connectionManager.forwardJoinInput <-- inputDispatcher.forwardJoinOutputs++;
        connectionManager.neighborInput <-- inputDispatcher.neighborOutputs++;
        connectionManager.disconnectInput <-- inputDispatcher.disconnectOutputs++;
        connectionManager.shuffleInput <-- inputDispatcher.shuffleOutputs++;
        connectionManager.shuffleReplyInput <-- inputDispatcher.shuffleReplyOutputs++;
        connectionManager.out --> outputFunnel.inputs++;
}
