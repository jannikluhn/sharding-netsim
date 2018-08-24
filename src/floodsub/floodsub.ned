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

    gates:
        input in;
        input sourceInput;
        input peerListChangeInput;
        output out;

    submodules:
        cache: Cache;
        flooder: Flooder {
            cachePath = "^.cache";
        };

    connections:
        in --> flooder.gossipInput;
        sourceInput --> flooder.sourceInput;

        flooder.peerListChangeInput <-- peerListChangeInput;

        flooder.out --> out;
}
