package sharding.mock_discoverer;

import sharding.IDiscoverer;
import sharding.utils.funnel.Funnel;


simple MockDiscoverer like IDiscoverer
{
    parameters:
        int nodeId;
        double datarate;

        int nodeCount;
        int minPeers;

    gates:
        input in;
        output out;
        output peerListChangeOutput;
}
