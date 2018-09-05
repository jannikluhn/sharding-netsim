package sharding.pushpull;

import sharding.IGossiper;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.cache.Cache;

import sharding.pushpull.puller.Puller;
import sharding.pushpull.pull_handler.PullHandler;


module PushPullGossiper like IGossiper
{
    parameters:
        int nodeId;
        double datarate;

        double startTime @unit(s);
        double period @unit(s);
        double requestInterval @unit(s);
        double pushTimeDelta;
        int pullFanout;

    gates:
        input in;
        input sourceInput;
        input peerListChangeInput;
        output out;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher {
            numPacketTypes = 2;
        };

        cache: Cache;

        puller: Puller {
            cachePath = "^.cache";

            startTime = startTime;
            period = period;
            requestInterval = requestInterval;
            pushTimeDelta = pushTimeDelta;
            pullFanout = pullFanout;
        };
        pull_handler: PullHandler {
            cachePath = "^.cache";
        };

    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        sourceInput --> puller.sourceInput;
        peerListChangeInput --> puller.peerListChangeInput;

        puller.gossipInput <-- inputDispatcher.outputs[0];
        puller.out --> outputFunnel.inputs++;

        pull_handler.pullInput <-- inputDispatcher.outputs[1];
        pull_handler.out --> outputFunnel.inputs++;
}
