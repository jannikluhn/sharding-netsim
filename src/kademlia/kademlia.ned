package sharding.kademlia;

import sharding.IDiscoverer;
import sharding.utils.funnel.Funnel;


module Kademlia like IDiscoverer
{
    parameters:
        int nodeId;
        double datarate;

        int shardId;

        int nodeCount;
        int minPeers;

    gates:
        input in;
        output out;
        output peerListChangeOutput;
}

