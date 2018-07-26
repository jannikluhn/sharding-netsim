#include <algorithm>
#include <omnetpp.h>
#include "peer_list_manager.h"

using namespace omnetpp;


Define_Module(PeerListManager);


void PeerListManager::addActivePeer(int peer_id) {
    if (isPeer(peer_id)) {
        error("%d is a peer already", peer_id);
    } else {
        active_peers.push_back(peer_id);
    }
}

void PeerListManager::addPassivePeer(int peer_id) {
    if (isPeer(peer_id)) {
        error("%d is a peer already", peer_id);
    } else {
        passive_peers.push_back(peer_id);
    }
}

void PeerListManager::activatePeer(int peer_id) {
    if (!isPeer(peer_id)) {
        error("%d is not a peer", peer_id);
    } else if (isActive(peer_id)) {
        error("Peer %d is already active", peer_id);
    } else {
        active_peers.push_back(peer_id);
        passive_peers.erase(
            std::remove(passive_peers.begin(), passive_peers.end(), peer_id)
        );
    }
}

void PeerListManager::passivatePeer(int peer_id) {
    if (!isPeer(peer_id)) {
        error("%d is not a peer", peer_id);
    } else if (isPassive(peer_id)) {
        error("Peer %d is already passive", peer_id);
    } else {
        passive_peers.push_back(peer_id);
        active_peers.erase(
            std::remove(active_peers.begin(), active_peers.end(), peer_id)
        );
    }
}

bool PeerListManager::isPeer(int peer_id) {
    return isActive(peer_id) || isPassive(peer_id);
}

bool PeerListManager::isActive(int peer_id) {
    return std::find(active_peers.begin(), active_peers.end(), peer_id) != active_peers.end();
}

bool PeerListManager::isPassive(int peer_id) {
    return std::find(passive_peers.begin(), passive_peers.end(), peer_id) != passive_peers.end();
}

int PeerListManager::getRandomPeer() {
    if (active_peers.empty() && passive_peers.empty()) {
        error("Peer list is empty");
    }

    int index = intuniform(0, active_peers.size() + passive_peers.size() - 1);
    if (index < active_peers.size()) {
        return active_peers[index];
    } else {
        return passive_peers[index];
    }
}

int PeerListManager::getRandomActivePeer() {
    if (active_peers.empty()) {
        error("Active list is empty");
    }
    int index = intuniform(0, active_peers.size() - 1);
    return active_peers[index];
}

int PeerListManager::getRandomPassivePeer() {
    if (passive_peers.empty()) {
        error("Passive list is empty");
    }
    int index = intuniform(0, passive_peers.size() - 1);
    return passive_peers[index];
}
