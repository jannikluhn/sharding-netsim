package sharding.episub;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;
import sharding.utils.cache.Cache;

import sharding.episub.gardener.Gardener;
import sharding.episub.gossip_handler.GossipHandler;
import sharding.episub.lazy_notifier.LazyNotifier;
import sharding.episub.missing_tracker.MissingTracker;
import sharding.episub.peer_tracker.PeerTracker;

import sharding.hyparview.ConnectionManager;


module EpiSubNode
{
    parameters:
        int nodeId;

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count);
        @statistic[missingRequested](record=count);

    gates:
        inout port;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        cache: Cache;
        source: Source {
            cachePath = "^.cache";
            nodeId = nodeId;
        };
        peerTracker: PeerTracker;
        gardener: Gardener {
            cachePath = "^.cache";
            peerTrackerPath = "^.peerTracker";
        };
        gossipHandler: GossipHandler {
            cachePath = "^.cache";
            peerTrackerPath = "^.peerTracker";
        };
        missingTracker: MissingTracker {
            cachePath = "^.cache";
        };

        connectionManager: ConnectionManager {
            nodeId = nodeId;
        };

    connections:
        port$i --> inputDispatcher.in;
        port$o <-- outputFunnel.out;

        source.outputs++ --> gossipHandler.internalGossipInput;

        gardener.graftInput <-- inputDispatcher.graftOutputs++;
        gardener.pruneInput <-- inputDispatcher.pruneOutputs++;
        gardener.out --> outputFunnel.inputs++;

        gossipHandler.externalGossipInput <-- inputDispatcher.gossipOutputs++;
        gossipHandler.out --> outputFunnel.inputs++;

        peerTracker.addedActivePeerInput <-- connectionManager.addedActivePeerOutput;
        peerTracker.removedActivePeerInput <-- connectionManager.removedActivePeerOutput;

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
