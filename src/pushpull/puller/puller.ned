simple Puller
{
    parameters:
        string cachePath;

        int pull_fanout;

        simtime_t startTime;
        double period;
        double requestInterval;
        double pushTimeDelta;

    gates:
        input pullInput;
        input peerListChangeInput;
        output out;
};
