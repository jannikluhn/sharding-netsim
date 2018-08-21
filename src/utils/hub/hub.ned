package sharding.utils.hub;


// Virtual hub through which all packets are routed.
simple Hub
{
    parameters:
        @signal[packetSent](long);

    gates:
        inout ports[];
}
