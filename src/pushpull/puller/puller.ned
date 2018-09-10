package sharding.pushpull.puller;


simple Puller
{
    parameters:
        string cachePath;

        int pushFanout;
        int pullFanout;

        double startTime @unit(s);
        double period @unit(s);
        double requestInterval @unit(s);
        double pushTimeDelta;

        @signal[newGossipReceived](type=simtime_t);

    gates:
        input sourceInput;
        input gossipInput;
        input peerListChangeInput;
        output out;
};
