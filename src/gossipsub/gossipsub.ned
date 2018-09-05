package sharding.gossipsub;

import sharding.IGossiper;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;
import sharding.utils.cache.Cache;

import sharding.gossipsub.overlay_manager.OverlayManager;
import sharding.gossipsub.gossiper.Gossiper;
import sharding.gossipsub.i_have_handler.IHaveHandler;
import sharding.gossipsub.i_want_handler.IWantHandler;


module GossipSubGossiper like IGossiper
{
    parameters:
        int nodeId;
        double datarate;

        double mbitPerPeer;
        double lowWatermarkFraction;
        double highWatermarkFraction;
        int maxPeers;

        double heartbeatInterval @unit(s);

        int targetMeshDegree = min(maxPeers, max(1, int(datarate / (mbitPerPeer * 1024 * 1024) + 0.5)));
        int lowWatermark = min(maxPeers, max(1, int(targetMeshDegree * lowWatermarkFraction + 0.5)));
        int highWatermark = min(maxPeers, max(1, int(targetMeshDegree * highWatermarkFraction + 0.5)));

        @statistic[peers](source=overlayChanged; record=last);

    gates:
        input in;
        input sourceInput;
        input peerListChangeInput;
        output out;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher {
            numPacketTypes = 5;
        };

        cache: Cache;

        gossiper: Gossiper {
            heartbeatInterval = heartbeatInterval;

            targetMeshDegree = targetMeshDegree;
            cachePath = "^.cache";
            overlayManagerPath = "^.overlayManager";
        };
        overlayManager: OverlayManager {
            heartbeatInterval = heartbeatInterval;

            targetMeshDegree = targetMeshDegree; 
            lowWatermark = lowWatermark; 
            highWatermark = highWatermark; 
        };
        i_have_handler: IHaveHandler {
            cachePath = "^.cache";
        };
        i_want_handler: IWantHandler {
            cachePath = "^.cache";
        };


    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        sourceInput --> gossiper.sourceInput;

        gossiper.externalGossipInput <-- inputDispatcher.outputs[0];
        gossiper.out --> outputFunnel.inputs++;

        overlayManager.graftInput <-- inputDispatcher.outputs[3];
        overlayManager.pruneInput <-- inputDispatcher.outputs[4];
        overlayManager.peerListChangeInput <-- peerListChangeInput;
        overlayManager.out --> outputFunnel.inputs++;

        i_have_handler.iHaveInput <-- inputDispatcher.outputs[1];
        i_have_handler.out --> outputFunnel.inputs++;

        i_want_handler.iWantInput <-- inputDispatcher.outputs[2];
        i_want_handler.out --> outputFunnel.inputs++;
}
