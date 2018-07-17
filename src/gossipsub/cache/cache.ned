package sharding.gossipsub.cache;


// Module that stores gossip.
simple Cache
{
    gates:
        input addGossipInputs[];
        output newGossipOutputs[];        
        output gardenerControlOutput;

        inout queryPorts[];
}
