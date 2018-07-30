package sharding.gossipsub.membership;

import sharding.gossipsub.funnel.Funnel;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;

import sharding.gossipsub.membership.peer_list.PeerList;
import sharding.gossipsub.membership.passive_list_manager.PassiveListManager;
import sharding.gossipsub.membership.get_nodes_handler.GetNodesHandler;
import sharding.gossipsub.membership.joiner.Joiner;


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
        joiner: Joiner {
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

          joiner.joinInput <-- inputDispatcher.joinOutputs++;
          joiner.neighborInput <-- inputDispatcher.neighborOutputs++;
          joiner.disconnectInput <-- inputDispatcher.disconnectOutputs++;
          joiner.forwardJoinInput <-- inputDispatcher.forwardJoinOutputs++;
          joiner.startInput <-- passiveListManager.viewInitializationFinishedOutputs++;
          joiner.out --> outputFunnel.inputs++;


//        joiner: Joiner;
//        getNodesHandler: GetNodesHandler;
//        joinHandler: GetNodesHandler;
//        activeListManager: ActiveListManager;
//        rttTracker: RTTTracker;
}
