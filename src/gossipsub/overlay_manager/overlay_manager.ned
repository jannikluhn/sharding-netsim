package sharding.gossipsub.overlay_manager;

simple OverlayManager
{
    parameters:
        double heartbeatInterval @unit(s);
        int targetMeshDegree;
        int lowWatermark;
        int highWatermark;

    gates:
        input addedActivePeerInput;
        input removedActivePeerInput;
        output out;
}
