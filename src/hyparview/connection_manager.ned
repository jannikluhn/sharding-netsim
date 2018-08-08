package sharding.hyparview;

import sharding.utils.funnel.Funnel;

import sharding.hyparview.peer_list.PeerList;
import sharding.hyparview.passive_list_manager.PassiveListManager;
import sharding.hyparview.get_nodes_handler.GetNodesHandler;
import sharding.hyparview.active_list_manager.ActiveListManager;


module ConnectionManager
{
    parameters:
        int nodeId;
        string contactNodes;

        int numRandomNeighbors;
        int numNearNeighbors;
        int passiveListSize;

        double shuffleInterval;
        int activeShufflingSize;
        int passiveShufflingSize;
        int shuffleTTL;

        double activeHeartbeatInterval;
        int joinTTL;
        int forwardJoinTTL;

        string peerListPath = "peerList";

        @statistic[passiveListUpdate](record=vector);
        @statistic[activeListUpdate](record=vector);


    gates:
        input getNodesInput;
        input nodesInput;
        input joinInput;
        input forwardJoinInput;
        input neighborInput;
        input disconnectInput;
        input shuffleInput;
        input shuffleReplyInput;

        output out;

        output addedActivePeerOutput;
        output removedActivePeerOutput;


    submodules:
        peerList: PeerList {
            parameters:
                nodeId = nodeId;
                contactNodes = contactNodes;
        };

        passiveListManager: PassiveListManager {
            parameters:
                nodeId = nodeId;
                peerListPath = "^." + peerListPath;

                shuffleInterval = shuffleInterval;
                activeShufflingSize = activeShufflingSize;
                passiveShufflingSize = passiveShufflingSize;
                shuffleTTL = shuffleTTL;
                passiveListSize = passiveListSize;
        };
        getNodesHandler: GetNodesHandler {
            parameters:
                nodeId = nodeId;
                peerListPath = "^." + peerListPath;
        };
        activeListManager: ActiveListManager {
            parameters:
                peerListPath = "^." + peerListPath;
                nodeId = nodeId;
                numRandomNeighbors = numRandomNeighbors;
                numNearNeighbors = numNearNeighbors;
                heartbeatInterval = activeHeartbeatInterval;
                joinTTL = joinTTL;
                forwardJoinTTL = forwardJoinTTL;
        };

        outputFunnel: Funnel;


    connections:
        outputFunnel.out --> out;

        passiveListManager.nodesInput <-- nodesInput;
        passiveListManager.shuffleInput <-- shuffleInput;
        passiveListManager.shuffleReplyInput <-- shuffleReplyInput;
        passiveListManager.forwardJoinInput <-- forwardJoinInput;
        passiveListManager.out --> outputFunnel.inputs++;

        getNodesHandler.getNodesInput <-- getNodesInput;
        getNodesHandler.out --> outputFunnel.inputs++;

        activeListManager.joinInput <-- joinInput;
        activeListManager.neighborInput <-- neighborInput;
        activeListManager.disconnectInput <-- disconnectInput;
        activeListManager.startInput <-- passiveListManager.viewInitializationFinishedOutputs++;
        activeListManager.out --> outputFunnel.inputs++;
        activeListManager.addedActivePeerOutput --> addedActivePeerOutput;
        activeListManager.removedActivePeerOutput --> removedActivePeerOutput;
}
