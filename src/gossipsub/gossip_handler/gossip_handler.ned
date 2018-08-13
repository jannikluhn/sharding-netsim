package sharding.gossipsub.gossip_handler;


simple GossipHandler
{
    parameters:
        string cachePath;
        string peerTrackerPath;

        double notificationInterval @unit(s);

        @signal[newGossipReceived](type=long);

    gates:
        input externalGossipInput;
        input internalGossipInput;
        output out;
}
