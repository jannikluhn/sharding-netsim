package sharding.gossipsub.gossiper;

simple Gossiper
{
    parameters:
        string cachePath;
        string overlayManagerPath;
        double heartbeatInterval @unit(s);
        int targetMeshDegree;

        @signal[newGossipReceived](type=simtime_t);

    gates:
        input sourceInput;
        input externalGossipInput;
        output out;
}
