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
        double gossipRate @unit(Hz);
        double startTime @unit(s);
        double stopTime @unit(s);
        bool active;

        double notificationInterval @unit(s);
        double requestWaitTime @unit(s);

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count);
        @statistic[missingRequested](record=count);

    gates:
        input in;
        input peerListChangeInput;
        output out;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher {
            numPacketTypes = 4;  // GOSSIP, IHAVE, GRAFT, PRUNE
        };

        cache: Cache;
        source: Source {
            gossipRate = gossipRate;
            startTime = startTime;
            stopTime = stopTime;
            active = active;
        };

        peerTracker: PeerTracker;
        gardener: Gardener {
            cachePath = "^.cache";
            peerTrackerPath = "^.peerTracker";
        };
        gossipHandler: GossipHandler {
            cachePath = "^.cache";
            peerTrackerPath = "^.peerTracker";
            notificationInterval = notificationInterval;
        };
        missingTracker: MissingTracker {
            cachePath = "^.cache";
            requestWaitTime = requestWaitTime;
        };

    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        source.outputs++ --> gossipHandler.internalGossipInput;

        gardener.graftInput <-- inputDispatcher.outputs[2];
        gardener.pruneInput <-- inputDispatcher.outputs[3];
        gardener.out --> outputFunnel.inputs++;

        gossipHandler.externalGossipInput <-- inputDispatcher.outputs[0];
        gossipHandler.out --> outputFunnel.inputs++;

        peerTracker.peerListChangeInput <-- peerListChangeInput;

        missingTracker.iHaveInput <-- inputDispatcher.outputs[1];
        missingTracker.out --> outputFunnel.inputs++;
}
