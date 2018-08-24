package sharding;

import sharding.utils.hub.Hub;
import sharding.utils.source.Source;
import sharding.utils.node_manager.NodeManager;


network Network
{
    parameters:
        double gossipRate @unit(Hz);
        bool periodic;
        double startTime @unit(s);
        double stopTime @unit(s);

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count,histogram);
        @statisticTemplate[packetSent](record=count);

    submodules:
        hub: Hub;
        nodeManager: NodeManager {
            hubPath = "^.hub";
            sourcePath = "^.source";
        };
        source: Source {
            gossipRate = gossipRate;
            periodic = periodic;
            startTime = startTime;
            stopTime = stopTime;
        };
}
