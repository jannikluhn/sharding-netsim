package sharding.gossipsub.gossiper;

simple Gossiper
{
    parameters:
        string cachePath;
        string overlayManagerPath;
        double heartbeatInterval @unit(s);
        int targetMeshDegree;

        @signal[newGossipReceived](type=long);

    gates:
        input internalGossipInput;
        input externalGossipInput;
        output out;
}
