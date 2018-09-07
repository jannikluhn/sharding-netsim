package sharding.utils.hub;


// Virtual hub through which all packets are routed.
simple Hub
{
    parameters:
        @signal[packetSent-*-*-*](type=bool);
        @statisticTemplate[packetSent](record=count);

        @signal[channelUsed-*](type=simtime_t);
        @statisticTemplate[channelUsed](record=last(sumPerDuration));

    gates:
        inout ports[];
}
