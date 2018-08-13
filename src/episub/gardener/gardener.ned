package sharding.episub.gardener;


// Module that prunes and grafts connections.
simple Gardener
{
    parameters:
        string cachePath;
        string peerTrackerPath;

    gates:
        input graftInput;
        input pruneInput;
        output out;
}
