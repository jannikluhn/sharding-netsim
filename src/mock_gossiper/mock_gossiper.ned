package sharding.mock_gossiper;

import sharding.IGossiper;
import sharding.IDiscoverer;

import sharding.utils.sink.Sink;


module MockGossiper like IGossiper
{
    parameters:
        int nodeId;

    gates:
        input in;
        input sourceInput;
        output out;
        input peerListChangeInput;

    submodules:
        sink: Sink;

    connections allowunconnected:
        in --> sink.inputs++;
        sourceInput --> sink.inputs++;
        peerListChangeInput --> sink.inputs++;
}
