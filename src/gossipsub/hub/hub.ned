package sharding.gossipsub.hub;


// Virtual hub through which all packets are routed.
simple Hub
{
    gates:
        inout ports[];
}
