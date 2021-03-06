package sharding.episub;

import sharding.IGossiper;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;
import sharding.utils.cache.Cache;

import sharding.episub.gardener.Gardener;
import sharding.episub.gossip_handler.GossipHandler;
import sharding.episub.lazy_notifier.LazyNotifier;
import sharding.episub.missing_tracker.MissingTracker;
import sharding.episub.peer_tracker.PeerTracker;


module EpiSubGossiper like IGossiper
{
    parameters:
        int nodeId;
        double datarate;

        double notificationInterval @unit(s);
        double requestWaitTime @unit(s);
        double requestRoundTripBound @unit(s);
        double minEagerTime @unit(s);

    gates:
        input in;
        input sourceInput;
        input peerListChangeInput;
        output out;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher {
            numPacketTypes = 4;  // GOSSIP, IHAVE, GRAFT, PRUNE
        };

        cache: Cache;

        peerTracker: PeerTracker;
        gardener: Gardener {
            cachePath = "^.cache";
            peerTrackerPath = "^.peerTracker";
        };
        gossipHandler: GossipHandler {
            cachePath = "^.cache";
            peerTrackerPath = "^.peerTracker";
            notificationInterval = notificationInterval;
            minEagerTime = minEagerTime;
        };
        missingTracker: MissingTracker {
            cachePath = "^.cache";
            requestWaitTime = requestWaitTime;
            requestRoundTripBound = requestRoundTripBound;
        };

    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        sourceInput --> gossipHandler.sourceInput;

        gardener.graftInput <-- inputDispatcher.outputs[2];
        gardener.pruneInput <-- inputDispatcher.outputs[3];
        gardener.out --> outputFunnel.inputs++;

        gossipHandler.externalGossipInput <-- inputDispatcher.outputs[0];
        gossipHandler.out --> outputFunnel.inputs++;

        peerTracker.peerListChangeInput <-- peerListChangeInput;

        missingTracker.iHaveInput <-- inputDispatcher.outputs[1];
        missingTracker.out --> outputFunnel.inputs++;
}
