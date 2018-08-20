#include <omnetpp.h>
#include "peer_tracker.h"
#include "../../packets_m.h"

using namespace omnetpp;


Define_Module(PeerTracker);


void PeerTracker::handleMessage(cMessage *msg) {
    PeerListChange *peer_list_change = check_and_cast<PeerListChange *>(msg);
    for (int i = 0; i < peer_list_change->getAddedPeersArraySize(); i++) {
        EV_DEBUG << "adding peer " << peer_list_change->getAddedPeers(i) << endl;
        addEager(peer_list_change->getAddedPeers(i));
    }
    for (int i = 0; i < peer_list_change->getRemovedPeersArraySize(); i++) {
        remove(peer_list_change->getRemovedPeers(i));
    }
    delete peer_list_change;
}

void PeerTracker::addEager(int node_id) {
    Enter_Method_Silent();
    eager_peers.insert(node_id);
    lazy_peers.erase(node_id);
}

void PeerTracker::makeEager(int node_id) {
    Enter_Method_Silent();
    if (!isPeer(node_id)) {
        error("Node is not a peer");    
    }
    eager_peers.insert(node_id);
    lazy_peers.erase(node_id);
}

void PeerTracker::makeLazy(int node_id) {
    Enter_Method_Silent();
    if (!isPeer(node_id)) {
        error("Node is not a peer");    
    }
    lazy_peers.insert(node_id);
    eager_peers.erase(node_id);
}

void PeerTracker::remove(int node_id) {
    Enter_Method_Silent();
    if (!isPeer(node_id)) {
        error("Node is not a peer");    
    }
    lazy_peers.erase(node_id);
    eager_peers.erase(node_id);
}

bool PeerTracker::isPeer(int node_id) {
    Enter_Method_Silent();
    return eager_peers.count(node_id) > 0 || lazy_peers.count(node_id) > 0;
}
