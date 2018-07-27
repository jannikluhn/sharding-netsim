package sharding.gossipsub.membership;

import sharding.gossipsub.funnel.Funnel;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;

import sharding.gossipsub.membership.peer_list.PeerList;
import sharding.gossipsub.membership.passive_list_manager.PassiveListManager;
import sharding.gossipsub.membership.get_nodes_handler.GetNodesHandler;


module ConnectionManager
{
    parameters:
        int nodeId;
        string contactNodes;

        int shuffleInterval;
        int activeShufflingSize;
        int passiveShufflingSize;
        int shuffleTTL;


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
        };
        getNodesHandler: GetNodesHandler {
            parameters:
                nodeId = nodeId;
                peerListPath = "^.peerList";
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


//        joiner: Joiner;
//        getNodesHandler: GetNodesHandler;
//        joinHandler: GetNodesHandler;
//        activeListManager: ActiveListManager;
//        rttTracker: RTTTracker;
}
