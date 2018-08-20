package sharding.floodsub;

import sharding.IGossiper;
import sharding.IDiscoverer;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;
import sharding.utils.cache.Cache;

import sharding.floodsub.flooder.Flooder;


module FloodSubGossiper like IGossiper
{
    parameters:
        int nodeId;
        double gossipRate @unit(Hz);
        double startTime @unit(s);
        double stopTime @unit(s);
        bool active;

    gates:
        input in;
        input peerListChangeInput;
        output out;

    submodules:
        cache: Cache;
        source: Source {
            gossipRate = gossipRate;
            startTime = startTime;
            stopTime = stopTime;
            active = active;
        };
        flooder: Flooder {
            cachePath = "^.cache";
        };

    connections:
        in --> flooder.gossipInput;
        source.outputs++ --> flooder.sourceInput;

        flooder.peerListChangeInput <-- peerListChangeInput;

        flooder.out --> out;
}
