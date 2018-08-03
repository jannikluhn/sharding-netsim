package sharding.gossipsub;

import sharding.utils.hub.Hub;
import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;

import sharding.gossipsub.source.Source;
import sharding.gossipsub.cache.Cache;
import sharding.gossipsub.gardener.Gardener;
import sharding.gossipsub.lazy_notifier.LazyNotifier;
import sharding.gossipsub.missing_tracker.MissingTracker;

import sharding.hyparview.ConnectionManager;


module GossipSubNode
{
    parameters:
        int nodeId;

    gates:
        inout port;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        source: Source {
            cachePath = "^.cache";
            nodeId = nodeId;
        };
        cache: Cache;
        gardener: Gardener {
            cachePath = "^.cache";
        };
        lazyNotifier: LazyNotifier;
        missingTracker: MissingTracker {
            cachePath = "^.cache";
        };

        connectionManager: ConnectionManager {
            nodeId = nodeId;
        };

    connections:
        port$i --> inputDispatcher.in;
        port$o <-- outputFunnel.out;

        source.outputs++ --> gardener.eagerMulticastInputs++;
        source.outputs++ --> lazyNotifier.gossipInputs++;

        gardener.graftInput <-- inputDispatcher.graftOutputs++;
        gardener.pruneInput <-- inputDispatcher.pruneOutputs++;
        gardener.gossipInput <-- inputDispatcher.gossipOutputs++;
        gardener.addedActivePeerInput <-- connectionManager.addedActivePeerOutput;
        gardener.removedActivePeerInput <-- connectionManager.removedActivePeerOutput;
        gardener.out --> outputFunnel.inputs++;

        lazyNotifier.notificationOutput --> gardener.lazyMulticastInputs++;

        missingTracker.iHaveInput <-- inputDispatcher.iHaveOutputs++;
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
