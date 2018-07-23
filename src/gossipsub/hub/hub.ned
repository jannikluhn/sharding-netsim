package sharding.gossipsub.hub;


// Virtual hub through which all packets are routed.
simple Hub
{
    parameters:
        @signal[messageSent](long);

    gates:
        inout ports[];
}
