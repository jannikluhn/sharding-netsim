package sharding.kademlia;

import sharding.IDiscoverer;
import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;

import sharding.kademlia.kademlia_peer_table.KademliaPeerTable;
import sharding.kademlia.kad_manager.KadManager;
import sharding.kademlia.kad_find_node_handler.KadFindNodeHandler;
import sharding.kademlia.kad_ping_handler.KadPingHandler;
import sharding.kademlia.kad_add_me_handler.KadAddMeHandler;


module Kademlia like IDiscoverer
{
    parameters:
        int nodeId;
        double datarate;

        int shardId;
        bool hidden;

        int nodeCount;
        int numBootnodes;
        int minPeers;
        int maxPeers;

        int lookupConcurrency;
        double maxLookupRoundDuration @unit(s);

    gates:
        input in;
        output out;
        output peerListChangeOutput;

    submodules:
        inputDispatcher: TypeDispatcher {
            numPacketTypes = 5;  // FIND_NODE, NEIGHBORS, PING, PONG, ADD_ME
        };
        outputFunnel: Funnel;

        peerTable: KademliaPeerTable {
            nodeId = nodeId;
            shardId = shardId;
        };
        manager: KadManager {
            peerTablePath = "^.peerTable";
            nodeId = nodeId;
            shardId = shardId;
            hidden = hidden;

            lookupConcurrency = lookupConcurrency;
            maxLookupRoundDuration = maxLookupRoundDuration;

            maxPeers = maxPeers;
            numBootnodes = numBootnodes;
        };

        findNodeHandler: KadFindNodeHandler {
            peerTablePath = "^.peerTable";
            shardId = shardId;
        };
        pingHandler: KadPingHandler {
            peerTablePath = "^.peerTable";
            shardId = shardId;
            hidden = hidden;
        };
        addMeHandler: KadAddMeHandler {
            peerTablePath = "^.peerTable";
        };

    connections:
        in --> inputDispatcher.in;
        out <-- outputFunnel.outputs++;

        manager.neighborsInput <-- inputDispatcher.outputs[1];
        manager.pongInput <-- inputDispatcher.outputs[3];
        manager.out --> outputFunnel.inputs++;
        manager.peerListChangeOutput --> peerListChangeOutput;

        findNodeHandler.findNodeInput <-- inputDispatcher.outputs[0];
        findNodeHandler.out --> outputFunnel.inputs++;

        pingHandler.pingInput <-- inputDispatcher.outputs[2];
        pingHandler.out --> outputFunnel.inputs++;

        addMeHandler.addMeInput <-- inputDispatcher.outputs[4];
}
