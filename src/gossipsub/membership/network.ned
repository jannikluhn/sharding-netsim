package sharding.gossipsub.membership;

import sharding.gossipsub.hub.Hub;

network TestNetwork {
    parameters:
        int nodeCount = 4;

    submodules:
        nodes[nodeCount]: ConnectionManager {
            parameters:
                nodeId = index;
                contactNodes = "0 1 2";
        };
        hub: Hub;

    connections:
        for i=0..nodeCount-1 {
            nodes[i].port <--> hub.ports++;
        }
        
}
