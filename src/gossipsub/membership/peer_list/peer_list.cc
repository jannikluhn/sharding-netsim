#include <algorithm>
#include <omnetpp.h>
#include "peer_list.h"

using namespace omnetpp;


Define_Module(PeerList);


void PeerList::initialize() {
    const char *contact_node_string = par("contactNodes").stringValue();
    std::vector<int> contact_node_indices = cStringTokenizer(contact_node_string).asIntVector();
    if (contact_node_indices.size() == 0) {
        error("Empty contact node list");
    }

    int my_id = getId();
    cModule *network = getModuleByPath(par("networkPath").stringValue());
    const char *node_name = par("nodeName").stringValue();
    for (auto contact_node_index : contact_node_indices) {
        int node_id = network->getSubmodule(node_name, contact_node_index)->getId();
        if (node_id != my_id) {
            addPassivePeer(node_id);
        }
    }
}

void PeerList::addActivePeer(int peer_id) {
    Enter_Method_Silent();

    if (isPeer(peer_id)) {
        error("%d is a peer already", peer_id);
    } else {
        active_peers.push_back(peer_id);
    }
}

void PeerList::addPassivePeer(int peer_id) {
    Enter_Method_Silent();

    if (isPeer(peer_id)) {
        error("%d is a peer already", peer_id);
    } else {
        passive_peers.push_back(peer_id);
    }
}

void PeerList::activatePeer(int peer_id) {
    Enter_Method_Silent();

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

void PeerList::passivatePeer(int peer_id) {
    Enter_Method_Silent();

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

bool PeerList::isPeer(int peer_id) {
    Enter_Method_Silent();

    return isActive(peer_id) || isPassive(peer_id);
}

bool PeerList::isActive(int peer_id) {
    Enter_Method_Silent();

    return std::find(active_peers.begin(), active_peers.end(), peer_id) != active_peers.end();
}

bool PeerList::isPassive(int peer_id) {
    Enter_Method_Silent();

    return std::find(passive_peers.begin(), passive_peers.end(), peer_id) != passive_peers.end();
}

int PeerList::getPeerListSize() {
    Enter_Method_Silent();

    return getActiveListSize() + getPassiveListSize();
}

int PeerList::getActiveListSize() {
    Enter_Method_Silent();

    return active_peers.size();
}

int PeerList::getPassiveListSize() {
    Enter_Method("getPassiveListSize");

    return passive_peers.size();
}

int PeerList::getPeerByIndex(int index) {
    Enter_Method_Silent();

    if (index < getActiveListSize()) {
        return getActivePeerByIndex(index);
    } else {
        return getPassivePeerByIndex(index);
    }
}

int PeerList::getActivePeerByIndex(int index) {
    Enter_Method_Silent();

    return active_peers[index];
}

int PeerList::getPassivePeerByIndex(int index) {
    Enter_Method_Silent();

    return passive_peers[index];
}

int PeerList::getRandomPeer() {
    Enter_Method_Silent();

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

int PeerList::getRandomActivePeer() {
    Enter_Method_Silent();

    if (active_peers.empty()) {
        error("Active list is empty");
    }
    int index = intuniform(0, active_peers.size() - 1);
    return active_peers[index];
}

int PeerList::getRandomPassivePeer() {
    Enter_Method_Silent();

    if (passive_peers.empty()) {
        error("Passive list is empty");
    }
    int index = intuniform(0, passive_peers.size() - 1);
    return passive_peers[index];
}
