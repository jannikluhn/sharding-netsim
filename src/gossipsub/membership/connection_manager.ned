package sharding.gossipsub.membership;

import sharding.gossipsub.funnel.Funnel;
import sharding.gossipsub.type_dispatcher.TypeDispatcher;

import sharding.gossipsub.membership.peer_list.PeerList;
import sharding.gossipsub.membership.passive_list_manager.PassiveListManager;
import sharding.gossipsub.membership.get_nodes_handler.GetNodesHandler;


module ConnectionManager
{
    parameters:
        string networkPath;
        string contactNodes;
        string nodeName;


    gates:
        inout port;


    submodules:
        peerList: PeerList {
            parameters:
                networkPath = "^." + networkPath;
                contactNodes = contactNodes;
                nodeName = nodeName;
        };

        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        passiveListManager: PassiveListManager {
            peerListPath = "^.peerList";
        };
        getNodesHandler: GetNodesHandler {
            peerListPath = "^.peerList";
        };



    connections:
          port$i --> inputDispatcher.in;
          port$o <-- outputFunnel.out;

          passiveListManager.nodesInput <-- inputDispatcher.nodesOutputs++;
          passiveListManager.out --> outputFunnel.inputs++;

          getNodesHandler.getNodesInput <-- inputDispatcher.getNodesOutputs++;
          getNodesHandler.out --> outputFunnel.inputs++;


//        joiner: Joiner;
//        getNodesHandler: GetNodesHandler;
//        joinHandler: GetNodesHandler;
//        activeListManager: ActiveListManager;
//        rttTracker: RTTTracker;
}
