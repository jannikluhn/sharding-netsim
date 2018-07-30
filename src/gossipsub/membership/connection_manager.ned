package sharding.gossipsub.membership;

import sharding.gossipsub.funnel.Funnel;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;

import sharding.gossipsub.membership.peer_list.PeerList;
import sharding.gossipsub.membership.passive_list_manager.PassiveListManager;
import sharding.gossipsub.membership.get_nodes_handler.GetNodesHandler;
import sharding.gossipsub.membership.active_list_manager.ActiveListManager;


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

        @statistic[passiveListUpdate](record=vector);
        @statistic[activeListUpdate](record=vector);


    gates:
        inout port;


    submodules:
        peerList: PeerList {
            parameters:
                nodeId = nodeId;
                contactNodes = contactNodes;
        };

        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

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
        active_list_manager: ActiveListManager {
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
          port$i --> inputDispatcher.in;
          port$o <-- outputFunnel.out;

          passiveListManager.nodesInput <-- inputDispatcher.nodesOutputs++;
          passiveListManager.shuffleInput <-- inputDispatcher.shuffleOutputs++;
          passiveListManager.shuffleReplyInput <-- inputDispatcher.shuffleReplyOutputs++;
          passiveListManager.out --> outputFunnel.inputs++;

          getNodesHandler.getNodesInput <-- inputDispatcher.getNodesOutputs++;
          getNodesHandler.out --> outputFunnel.inputs++;

          active_list_manager.joinInput <-- inputDispatcher.joinOutputs++;
          active_list_manager.neighborInput <-- inputDispatcher.neighborOutputs++;
          active_list_manager.disconnectInput <-- inputDispatcher.disconnectOutputs++;
          active_list_manager.forwardJoinInput <-- inputDispatcher.forwardJoinOutputs++;
          active_list_manager.startInput <-- passiveListManager.viewInitializationFinishedOutputs++;
          active_list_manager.out --> outputFunnel.inputs++;
}
