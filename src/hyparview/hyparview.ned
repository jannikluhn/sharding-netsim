package sharding.hyparview;

import sharding.IDiscoverer;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;

import sharding.hyparview.peer_list.PeerList;
import sharding.hyparview.passive_list_manager.PassiveListManager;
import sharding.hyparview.get_nodes_handler.GetNodesHandler;
import sharding.hyparview.active_list_manager.ActiveListManager;


module HyParViewDiscoverer like IDiscoverer
{
    parameters:
        int nodeId;
        double datarate;
        string contactNodes;

        int numRandomNeighbors;
        int numNearNeighbors;
        int passiveListSize;

        double shuffleInterval @unit(s);
        int activeShufflingSize;
        int passiveShufflingSize;
        int shuffleTTL;

        double activeHeartbeatInterval @unit(s);
        int joinTTL;
        int forwardJoinTTL;

    gates:
        input in;
        output out;
        output peerListChangeOutput;

    submodules:
        outputFunnel: Funnel;
        typeDispatcher: TypeDispatcher {
            numPacketTypes = 8;
        };

        peerList: PeerList {
            parameters:
                nodeId = nodeId;
                contactNodes = contactNodes;
        };

        passiveListManager: PassiveListManager {
            parameters:
                nodeId = nodeId;
                peerListPath = "^.peerList";

                shuffleInterval = shuffleInterval;
                activeShufflingSize = activeShufflingSize;
                passiveShufflingSize = passiveShufflingSize;
                shuffleTTL = shuffleTTL;
                passiveListSize = passiveListSize;
        };
        getNodesHandler: GetNodesHandler {
            parameters:
                nodeId = nodeId;
                peerListPath = "^.peerList";
        };
        activeListManager: ActiveListManager {
            parameters:
                peerListPath = "^.peerList";
                nodeId = nodeId;
                numRandomNeighbors = numRandomNeighbors;
                numNearNeighbors = numNearNeighbors;
                heartbeatInterval = activeHeartbeatInterval;
                joinTTL = joinTTL;
                forwardJoinTTL = forwardJoinTTL;
        };

    connections:
        in --> typeDispatcher.in;
        outputFunnel.outputs++ --> out;

        passiveListManager.nodesInput <-- typeDispatcher.outputs[1];
        passiveListManager.shuffleInput <-- typeDispatcher.outputs[6];
        passiveListManager.shuffleReplyInput <-- typeDispatcher.outputs[7];
        passiveListManager.forwardJoinInput <-- typeDispatcher.outputs[3];
        passiveListManager.out --> outputFunnel.inputs++;

        getNodesHandler.getNodesInput <-- typeDispatcher.outputs[0];
        getNodesHandler.out --> outputFunnel.inputs++;

        activeListManager.joinInput <-- typeDispatcher.outputs[2];
        activeListManager.neighborInput <-- typeDispatcher.outputs[4];
        activeListManager.disconnectInput <-- typeDispatcher.outputs[5];
        activeListManager.startInput <-- passiveListManager.viewInitializationFinishedOutputs++;
        activeListManager.out --> outputFunnel.inputs++;
        activeListManager.activeListChangeOutput --> peerListChangeOutput;
}
