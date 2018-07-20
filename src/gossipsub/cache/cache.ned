package sharding.gossipsub.cache;


// Module that stores gossip.
simple Cache
{
    parameters:
        @signal[newGossipReceived](long);

    gates:
        input addGossipInputs[];
        output newGossipOutputs[];        
        output gardenerControlOutput;

        inout queryPorts[];
}
