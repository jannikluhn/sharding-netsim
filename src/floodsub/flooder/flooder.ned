package sharding.floodsub.flooder;

simple Flooder
{
    parameters:
        string cachePath;

        @signal[newGossipReceived](type=simtime_t);

    gates:
        input sourceInput;
        input gossipInput;
        input peerListChangeInput;
        output out;
}
