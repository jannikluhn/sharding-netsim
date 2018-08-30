package sharding;

import sharding.interfaces.IGossiper;
import sharding.interfaces.IDiscoverer;
import sharding.utils.queue.Queue;
import sharding.utils.funnel.Funnel;
import sharding.utils.protocol_dispatcher.ProtocolDispatcher;


module Node
{
    parameters:
        int nodeId;

        string gossiperType;
        string discovererType;

    gates:
        inout port;
        input sourceInput;

    submodules:
        queue: Queue;
        outputFunnel: Funnel;

        protocolDispatcher: ProtocolDispatcher;

        gossiper: <gossiperType> like IGossiper {
            nodeId = nodeId;
        }
        discoverer: <discovererType> like IDiscoverer {
            nodeId = nodeId;
        }

    connections:
        port$i --> protocolDispatcher.in;
        protocolDispatcher.gossipOutput --> gossiper.in;
        protocolDispatcher.discoveryOutput --> discoverer.in;

        sourceInput --> gossiper.sourceInput;
        discoverer.peerListChangeOutput --> gossiper.peerListChangeInput;

        gossiper.out --> outputFunnel.inputs++;
        discoverer.out --> outputFunnel.inputs++;
        outputFunnel.outputs++ --> queue.in;
        queue.out --> port$o;
}
