#include <omnetpp.h>
#include "./mock_discoverer_packets_m.h"
#include "../packets_m.h"
#include "mock_discoverer.h"

using namespace omnetpp;


Define_Module(MockDiscoverer);

int MockDiscoverer::numInitStages() const {
    return 2;
}

void MockDiscoverer::initialize(int stage) {
    if (stage == 0) {
        return;  // wait until all bootnodes are created
    } else if (stage == 1) {
        int node_id = par("nodeId").intValue();
        int node_count = par("nodeCount").intValue();
        int min_peers = par("minPeers").intValue();

        if (min_peers > node_count - 1) {
            error("Not enough nodes in the network");
        }

        EV_DEBUG << "looking for " << min_peers << " peers" << endl;
        while (peers.size() < min_peers) {
            int potential_peer = intuniform(0, node_count - 1);
            if (potential_peer != node_id) {
                peers.insert(potential_peer);
            }
        }
        EV_DEBUG << "peer discovery finished" << endl;

        for (auto peer : peers) {
            ForceJoin *join = new ForceJoin();
            join->setReceiver(peer);
            send(join, "out"); 
        }


        PeerListChange *peer_list_change = new PeerListChange();
        peer_list_change->setAddedPeersArraySize(peers.size());
        int i = 0;
        for (auto peer : peers) {
            peer_list_change->setAddedPeers(i, peer);
            i++;
        }
        send(peer_list_change, "peerListChangeOutput");
    }
}

void MockDiscoverer::handleMessage(cMessage *msg) {
    ForceJoin *join = check_and_cast<ForceJoin *>(msg);
    int peer = join->getSender();
    if (peers.count(peer) == 0) {
        peers.insert(peer);
        EV_DEBUG << "Accepted connection with " << peer << " (got " << peers.size()
            << " peers now)" << endl;

        PeerListChange *peer_list_change = new PeerListChange();
        peer_list_change->setAddedPeersArraySize(1);
        peer_list_change->setAddedPeers(0, peer);
        send(peer_list_change, "peerListChangeOutput");
    }
    delete join;
}
