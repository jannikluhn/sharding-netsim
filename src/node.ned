package sharding;

import sharding.interfaces.IGossiper;
import sharding.interfaces.IDiscoverer;
import sharding.utils.queue.Queue;
import sharding.utils.funnel.Funnel;
import sharding.utils.crash_switch.CrashSwitch;
import sharding.utils.protocol_dispatcher.ProtocolDispatcher;


module Node
{
    parameters:
        int nodeId;
        double datarate;
        volatile bool crashed = default(false);

        string gossiperType;
        string discovererType;

        @statistic[peerListUpdate](record=last);

    gates:
        inout port;
        input sourceInput;

    submodules:
        queue: Queue;
        outputFunnel: Funnel;

        crashSwitch: CrashSwitch {
            crashed = crashed;
        };
        protocolDispatcher: ProtocolDispatcher;

        gossiper: <gossiperType> like IGossiper {
            nodeId = nodeId;
            datarate = datarate;
        }
        discoverer: <discovererType> like IDiscoverer {
            nodeId = nodeId;
            datarate = datarate;
        }

    connections:
        port$i --> crashSwitch.in;
        crashSwitch.out --> protocolDispatcher.in;
        protocolDispatcher.gossipOutput --> gossiper.in;
        protocolDispatcher.discoveryOutput --> discoverer.in;

        sourceInput --> crashSwitch.sourceInput;
        crashSwitch.sourceOutput --> gossiper.sourceInput;

        discoverer.peerListChangeOutput --> gossiper.peerListChangeInput;

        gossiper.out --> outputFunnel.inputs++;
        discoverer.out --> outputFunnel.inputs++;
        outputFunnel.outputs++ --> queue.in;
        queue.out --> port$o;
}
