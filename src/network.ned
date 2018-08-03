package sharding;

import sharding.gossipsub.GossipSubNode;
import sharding.utils.hub.Hub;


network Network
{
    parameters:
        int nodeCount;
        double gossipRate @unit(Hz);

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count,histogram);
        @statistic[messageSent](record=count);

    submodules:
        nodes[nodeCount]: GossipSubNode {
            nodeId = index;
            source.rate = gossipRate / nodeCount;
        }
        hub: Hub;

    connections:
        for i=0..sizeof(nodes)-1 {
            nodes[i].port <--> { delay=100ms; } <--> hub.ports++;
        }
}
