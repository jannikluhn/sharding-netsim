package sharding.floodsub;

import sharding.utils.funnel.Funnel;
import sharding.utils.type_dispatcher.TypeDispatcher;
import sharding.utils.source.Source;
import sharding.utils.cache.Cache;

import sharding.hyparview.ConnectionManager;

import sharding.floodsub.flooder.Flooder;


module FloodSubNode
{
    parameters:
        int nodeId;

    gates:
        inout port;

    submodules:
        outputFunnel: Funnel;
        inputDispatcher: TypeDispatcher;

        cache: Cache;
        source: Source {
            cachePath = "^.cache";
            nodeId = nodeId;
        };
        flooder: Flooder {
            cachePath = "^.cache";
        };

        connectionManager: ConnectionManager {
            nodeId = nodeId;
        };


    connections:
        port$i --> inputDispatcher.in;
        port$o <-- outputFunnel.out;

        source.outputs++ --> flooder.sourceInput;

        flooder.gossipInput <-- inputDispatcher.gossipOutputs++;
        flooder.addedActivePeerInput <-- connectionManager.addedActivePeerOutput;
        flooder.removedActivePeerInput <-- connectionManager.removedActivePeerOutput;
        flooder.out --> outputFunnel.inputs++;

        connectionManager.getNodesInput <-- inputDispatcher.getNodesOutputs++;
        connectionManager.nodesInput <-- inputDispatcher.nodesOutputs++;
        connectionManager.joinInput <-- inputDispatcher.joinOutputs++;
        connectionManager.forwardJoinInput <-- inputDispatcher.forwardJoinOutputs++;
        connectionManager.neighborInput <-- inputDispatcher.neighborOutputs++;
        connectionManager.disconnectInput <-- inputDispatcher.disconnectOutputs++;
        connectionManager.shuffleInput <-- inputDispatcher.shuffleOutputs++;
        connectionManager.shuffleReplyInput <-- inputDispatcher.shuffleReplyOutputs++;
        connectionManager.out --> outputFunnel.inputs++;
}
