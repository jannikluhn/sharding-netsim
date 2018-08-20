package sharding.mock_gossiper;

import sharding.IGossiper;
import sharding.IDiscoverer;

import sharding.utils.sink.Sink;


module MockGossiper like IGossiper
{
    parameters:
        int nodeId;
        double gossipRate @unit(Hz);

    gates:
        input in;
        output out;
        input peerListChangeInput;

    submodules:
        sink: Sink;

    connections allowunconnected:
        in --> sink.inputs++;
        peerListChangeInput --> sink.inputs++;
}
