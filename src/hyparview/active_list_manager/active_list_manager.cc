#include <omnetpp.h>
#include "active_list_manager.h"

using namespace omnetpp;


Define_Module(ActiveListManager);


void ActiveListManager::initialize() {
    const char *peer_list_path = par("peerListPath").stringValue();
    peer_list = check_and_cast<PeerList *>(getModuleByPath(peer_list_path));

    node_id = par("nodeId").intValue();
    num_random_neighbors = par("numRandomNeighbors").intValue();
    num_near_neighbors = par("numNearNeighbors").intValue();
    num_neighbors = num_random_neighbors + num_near_neighbors;
    join_ttl = par("joinTTL").intValue();
    forward_join_ttl = par("forwardJoinTTL").intValue();
    heartbeat_interval = par("heartbeatInterval").doubleValue();

    is_heart_beating = false;

    peer_list_update_signal = registerSignal("peerListUpdate");
    active_list_update_signal = registerSignal("activeListUpdate");
}

void ActiveListManager::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("joinInput")) {
        handleJoin(check_and_cast<Join *>(msg));
    } else if (msg->arrivedOn("neighborInput")) {
        handleNeighbor(check_and_cast<Neighbor *>(msg));
    } else if (msg->arrivedOn("disconnectInput")) {
        handleDisconnect(check_and_cast<Disconnect *>(msg));
    } else if (msg->arrivedOn("startInput")) {
        sendInitialJoins();
        delete msg;
    }
}

void ActiveListManager::sendInitialJoins() {
    emit(active_list_update_signal, peer_list->getActiveListSize());

    int num_receivers = std::min(num_random_neighbors, peer_list->getPassiveListSize());
    if (num_receivers == 0) {
        error("No contact nodes to join with");
    }

    EV_DEBUG << "sending initial joins to " << num_receivers << " nodes" << endl;

    std::vector<int> shuffling = peer_list->getPassiveListShuffling();
    std::vector<int> receivers(shuffling.begin(), shuffling.begin() + num_receivers);

    Join *join_template = new Join();
    join_template->setTtl(join_ttl);
    join_template->setNode(node_id);
    for (auto receiver : receivers) {
        Join *join = join_template->dup();
        join->setReceiver(receiver);
        send(join, "out");
    }
    delete join_template;
}

void ActiveListManager::startHeartbeat() {
    scheduleAt(simTime() + uniform(0, heartbeat_interval), new cMessage());
}

void ActiveListManager::handleHeartbeat(cMessage *heartbeat) {
    scheduleAt(simTime() + heartbeat_interval, heartbeat);

    int active_list_size = peer_list->getActiveListSize();
    if (active_list_size > num_neighbors) {
        // disconnect from random peer
        int peer = peer_list->getRandomActivePeer();
        EV_DEBUG << "Too many active peers (" << active_list_size << " > " << num_neighbors
            << "), disconnecting from " << peer << endl;

        Disconnect *disconnect = new Disconnect();
        disconnect->setReceiver(peer);
        send(disconnect, "out");

        peer_list->passivatePeer(peer);
        emit(active_list_update_signal, peer_list->getActiveListSize());

        PeerListChange *peer_list_update = new PeerListChange();
        peer_list_update->setRemovedPeersArraySize(1);
        peer_list_update->setRemovedPeers(0, peer);
        send(peer_list_update, "activeListChangeOutput");
    } else if (active_list_size < num_neighbors && neighbor_requests.size() == 0) {
        // connect to random passive peer
        if (peer_list->getPassiveListSize() > 0) {
            int peer = peer_list->getRandomPassivePeer();
            EV_DEBUG << "Too few active peers (" << active_list_size << " < " << num_neighbors
                << "), connecting to " << peer << endl;

            Neighbor *neighbor = new Neighbor();
            neighbor->setReceiver(peer);
            send(neighbor, "out");

            neighbor_requests.insert(peer);
        } else {
            EV_WARN << "Passive list empty, but additional active peers needed ("
                << active_list_size << " < " << num_neighbors << ")" << endl;
        }
    }
}

void ActiveListManager::handleJoin(Join *join) {
    int ttl = join->getTtl();
    int node = join->getNode();

    if (ttl > 0 && peer_list->getActiveListSize() >= num_neighbors) {
        EV_DEBUG << "forwarding received join" << endl;

        // forward join to random active peer
        join->setTtl(ttl - 1);
        do {
            join->setReceiver(peer_list->getRandomActivePeer());
        } while (join->getReceiver() == join->getSender());

        send(join, "out");
    } else if (peer_list->isActive(node)) {
        EV_DEBUG << "received join from already active peer, forwarding" << endl;

        // forward join to random active peer, without decrementing TTL
        do {
            join->setReceiver(peer_list->getRandomActivePeer());
        } while (join->getReceiver() == join->getSender());

        send(join, "out");
    } else {
        EV_DEBUG << "accepting join from " << node << endl;

        // accept join
        Neighbor *neighbor = new Neighbor();
        neighbor->setReceiver(node);
        neighbor_requests.insert(node);
        send(neighbor, "out");

        if (peer_list->getActiveListSize() > 0) {
            ForwardJoin *forward_join = new ForwardJoin();
            forward_join->setReceiver(peer_list->getRandomActivePeer());
            forward_join->setTtl(forward_join_ttl);
            forward_join->setNode(node);
            send(forward_join, "out");
        }

        delete join;
    }
}

void ActiveListManager::handleNeighbor(Neighbor *neighbor) {
    // TODO: accept if peer has too few neighbors and if it is "near"
    int node = neighbor->getSender();

    if (neighbor_requests.count(node) > 0) {
        // if we requested to connect just add the peer if we haven't already
        neighbor_requests.erase(node);
        acceptNeighborRequest(node);

        EV_DEBUG << "received positive response to neighbor request from " << node << " ("
            << peer_list->getActiveListSize() << " peers)" << endl;
    } else {
        // if they requested check if we want to connect and send reply accordingly
        if (peer_list->getActiveListSize() < num_neighbors) {
            acceptNeighborRequest(node);
            EV_DEBUG << "accepted neighbor request from " << node << " ("
                << peer_list->getActiveListSize() << " peers)" << endl;

            Neighbor *reply = new Neighbor();
            reply->setReceiver(node);
            send(reply, "out");
        } else {
            EV_DEBUG << "rejected neighbor request by " << node << " ("
                << peer_list->getActiveListSize() << " peers)" << endl;

            // reject
            Disconnect *reply = new Disconnect();
            reply->setReceiver(node);
            send(reply, "out");
        }
    }

    delete neighbor;

    if (!is_heart_beating) {
        is_heart_beating = true;
        startHeartbeat();
    }
}

void ActiveListManager::acceptNeighborRequest(int node) {
    if (peer_list->isActive(node)) {
        EV_DEBUG << "peer " << node << " already active" << endl;
        return;
    }

    if (peer_list->isPassive(node)) {
        peer_list->activatePeer(node);
    } else {
        // peer is completely new
        peer_list->addActivePeer(node);
        emit(peer_list_update_signal, peer_list->getPeerListSize());
    }

    emit(active_list_update_signal, peer_list->getActiveListSize());

    PeerListChange *peer_list_change = new PeerListChange();
    peer_list_change->setAddedPeersArraySize(1);
    peer_list_change->setAddedPeers(0, node);
    send(peer_list_change, "activeListChangeOutput");
}

void ActiveListManager::handleDisconnect(Disconnect *disconnect) {
    int peer_id = disconnect->getSender();
    if (peer_list->isActive(peer_id)) {
        EV_DEBUG << "received disconnect request from " << peer_id << ", passivating peer ("
            << peer_list->getActiveListSize() - 1 << " peers)" << endl;
        peer_list->passivatePeer(peer_id);
        emit(active_list_update_signal, peer_list->getActiveListSize());

        PeerListChange *peer_list_change = new PeerListChange();
        peer_list_change->setRemovedPeersArraySize(1);
        peer_list_change->setRemovedPeers(0, peer_id);
        send(peer_list_change, "activeListChangeOutput");
    } else if (neighbor_requests.count(peer_id) > 0) {
        EV_DEBUG << "neighbor request rejected by " << peer_id << " ("
            << peer_list->getActiveListSize() << " peers)" << endl;
        neighbor_requests.erase(peer_id);
    } else {
        // may happen if two nodes disconnect from each other at the same time
        EV_DEBUG << "received disconnect request from already unconnected peer " << peer_id
            << endl;
    }
    delete disconnect;
}
