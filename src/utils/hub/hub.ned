package sharding.utils.hub;


// Virtual hub through which all packets are routed.
simple Hub
{
    parameters:
        @signal[packetSent-*-*-*](type=bool);
        @statisticTemplate[packetSent](record=count);

    gates:
        inout ports[];
}
