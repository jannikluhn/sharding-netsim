package sharding;

import sharding.floodsub.FloodSubNode;
import sharding.gossipsub.GossipSubNode;
import sharding.utils.hub.Hub;
import sharding.utils.node_manager.NodeManager;


network Network
{
    parameters:
        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count);

    submodules:
        hub: Hub;
        nodeManager: NodeManager {
            hubPath = "^.hub";
        };
}
