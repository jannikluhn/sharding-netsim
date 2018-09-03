package sharding.episub.gossip_handler;


simple GossipHandler
{
    parameters:
        string cachePath;
        string peerTrackerPath;

        double notificationInterval @unit(s);
        double minEagerTime @unit(s);

        @signal[newGossipReceived](type=simtime_t);

    gates:
        input externalGossipInput;
        input sourceInput;
        output out;
}
