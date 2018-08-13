package sharding.gossipsub;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;

import sharding.hyparview.ConnectionManager;

import sharding.gossipsub.overlay_manager.OverlayManager;


module GossipSubNode
{
    parameters:
        int nodeId;

        int targetMeshDegree;
        int lowWatermark;
        int highWatermark;

        double heartbeatInterval @unit(s);

        //@statistic[newGossipEmitted](record=count);
        //@statistic[newGossipReceived](record=count);
        @statistic[activeListUpdate](record=last);
        @statistic[peerListUpdate](record=last);

    gates:
        inout port;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        source: Source;

        overlayManager: OverlayManager {
            heartbeatInterval = heartbeatInterval;

            targetMeshDegree = targetMeshDegree; 
            lowWatermark = lowWatermark; 
            highWatermark = highWatermark; 
        };

        connectionManager: ConnectionManager {
            nodeId = nodeId;
        };

    connections:
        port$i --> inputDispatcher.in;
        port$o <-- outputFunnel.out;

        overlayManager.addedActivePeerInput <-- connectionManager.addedActivePeerOutput;
        overlayManager.removedActivePeerInput <-- connectionManager.removedActivePeerOutput;
        overlayManager.out --> outputFunnel.inputs++;

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
