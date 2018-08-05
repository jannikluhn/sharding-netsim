package sharding.floodsub.flooder;

simple Flooder
{
    parameters:
        string cachePath;

        @signal[newGossipReceived](type=long);

    gates:
        input sourceInput;
        input gossipInput;
        input addedActivePeerInput;
        input removedActivePeerInput;
        output out;
}
