package sharding.kademlia;

import sharding.IDiscoverer;
import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;

import sharding.kademlia.kademlia.KademliaPeerTable;
import sharding.kademlia.kad_manager.KadManager;
import sharding.kademlia.kad_find_node_handler.KadFindNodeHandler;
import sharding.kademlia.kad_ping_handler.KadPingHandler;


module Kademlia like IDiscoverer
{
    parameters:
        int nodeId;
        double datarate;

        bool hidden;

        int nodeCount;
        int numBootnodes;
        int minPeers;

        int lookupConcurrency;
        double maxLookupRoundDuration @unit(s);
        int lookupInterval @unit(s);

    gates:
        input in;
        output out;
        output peerListChangeOutput;

    submodules:
        inputDispatcher: TypeDispatcher {
            numPacketTypes = 5;  // FIND_NODE, NEIGHBORS, PING, PONG, ADD_ME
        };
        outputFunnel: Funnel;

        peerTable: KademliaPeerTable;
        manager: KadManager {
            peerTablePath = "^.peerTable";
            nodeId = nodeId;
            hidden = hidden;

            lookupConcurrency = lookupConcurrency;
            maxLookupRoundDuration = maxLookupRoundDuration;
            lookupInterval = lookupInterval;

            numBootnodes = numBootnodes;
        };

        findNodeHandler: KadFindNodeHandler {
            peerTablePath = "^.peerTable";
        };
        pingHandler: KadPingHandler {
            peerTablePath = "^.peerTable";
            hidden = hidden;
        };

    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        manager.neighborsInput <-- inputDispatcher.outputs[1];
        manager.pongInput <-- inputDispatcher.outputs[3];
        manager.addMeInput <-- inputDispatcher.outputs[4];
        manager.out --> outputFunnel.inputs++;
        manager.peerListChangeOutput --> peerListChangeOutput;

        findNodeHandler.findNodeInput <-- inputDispatcher.outputs[0];
        findNodeHandler.out --> outputFunnel.inputs++;

        pingHandler.pingInput <-- inputDispatcher.outputs[2];
        pingHandler.out --> outputFunnel.inputs++;
}
