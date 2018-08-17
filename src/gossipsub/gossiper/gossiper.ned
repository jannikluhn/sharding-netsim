package sharding.gossipsub.gossiper;

simple Gossiper
{
    parameters:
        string cachePath;
        string overlayManagerPath;
        double heartbeatInterval @unit(s);
        int targetMeshDegree;

    gates:
        input internalGossipInput;
        input externalGossipInput;
        output out;
}
