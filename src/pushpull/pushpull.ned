package sharding.gossipsub;

import sharding.IGossiper;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;
import sharding.utils.cache.Cache;

import sharding.pushpull.puller.Puller;
import sharding.pushpull.pull_handler.PullHandler;


module PushPullGossiper like IGossiper
{
    parameters:
        int nodeId;

        simtime_t startTime @unit(s);
        double period @unit(s);

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

        puller: Puller {
            startTime = startTime;
            period = period;
        };
        pull_handler: PullHandler {
            cachePath = "^.cache";
        };

    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        sourceInput --> gossiper.sourceInput;

        puller.externalGossipInput <-- inputDispatcher.outputs[0];
        puller.out --> outputFunnel.inputs++;

        pull_handler.pullInput <-- inputDispatcher.outputs[1];
        pull_handler.out --> outputFunnel.inputs++;
}
