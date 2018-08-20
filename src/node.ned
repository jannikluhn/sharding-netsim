package sharding;

import sharding.interfaces.IGossiper;
import sharding.interfaces.IDiscoverer;
import sharding.utils.funnel.Funnel;
import sharding.utils.protocol_dispatcher.ProtocolDispatcher;


module Node
{
    parameters:
        int nodeId;

        string gossiperType;
        string discovererType;

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count);

    gates:
        inout port;

    submodules:
        inputFunnel: Funnel;
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

        discoverer.peerListChangeOutput --> gossiper.peerListChangeInput;

        gossiper.out --> outputFunnel.inputs++;
        discoverer.out --> outputFunnel.inputs++;
        outputFunnel.outputs++ --> port$o;
}
