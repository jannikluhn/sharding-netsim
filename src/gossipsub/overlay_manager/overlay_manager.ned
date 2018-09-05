package sharding.gossipsub.overlay_manager;

simple OverlayManager
{
    parameters:
        double heartbeatInterval @unit(s);
        int targetMeshDegree;
        int lowWatermark;
        int highWatermark;

        @signal[overlayChanged](type=unsigned long);

    gates:
        input graftInput;
        input pruneInput;
        input peerListChangeInput;
        output out;
}
