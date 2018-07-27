package sharding.gossipsub.membership;

import sharding.gossipsub.hub.Hub;

network TestNetwork {
    parameters:
        int nodeCount;

    submodules:
        nodes[nodeCount]: ConnectionManager {
            parameters:
                nodeId = index;
        };
        hub: Hub;

    connections:
        for i=0..nodeCount-1 {
            nodes[i].port <--> hub.ports++;
        }
        
}
