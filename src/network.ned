package sharding;

import sharding.floodsub.FloodSubNode;
import sharding.gossipsub.GossipSubNode;
import sharding.utils.hub.Hub;
import sharding.utils.node_manager.NodeManager;


network Network
{
    submodules:
        hub: Hub;
        nodeManager: NodeManager {
            hubPath = "^.hub";
            nodeTypeName = "sharding.floodsub.FloodSubNode";
        };
}


network FloodSubNetwork
{
    parameters:
        int nodeCount;
        double gossipRate @unit(Hz);

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count,histogram);
        @statistic[messageSent](record=count);

    submodules:
        nodes[nodeCount]: FloodSubNode {
            nodeId = index;
            source.rate = gossipRate / nodeCount;
        }
        hub: Hub;

    connections:
        for i=0..sizeof(nodes)-1 {
            nodes[i].port <--> { delay=uniform(30ms, 200ms); } <--> hub.ports++;
        }
}


network GossipSubNetwork
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
            nodes[i].port <--> { delay=uniform(30ms, 200ms); } <--> hub.ports++;
        }
}
