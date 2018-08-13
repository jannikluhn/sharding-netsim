package sharding.gossipsub.peer_tracker;


// Module that keeps track of which peers are eager and which are lazy.
simple PeerTracker
{
    gates:
        input addedActivePeerInput;
        input removedActivePeerInput;
}
