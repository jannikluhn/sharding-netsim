#include <omnetpp.h>
#include "active_list_manager.h"
#include "../../packets_m.h"
#include "../internal_messages_m.h"

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

        ActiveListChange *active_list_change = new ActiveListChange();
        active_list_change->setRemoved(peer);
        send(active_list_change, "removedActivePeerOutput");
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
        EV_DEBUG << "accepting join" << endl;

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
        if (peer_list->isPassive(node)) {
            peer_list->activatePeer(node);
        } else if (!peer_list->isActive(node)) {
            peer_list->addActivePeer(node);
        }

        int active_list_size = peer_list->getActiveListSize();
        emit(active_list_update_signal, active_list_size);
        EV_DEBUG << "received positive response to neighbor request from " << node << " ("
            << active_list_size << " peers)" << endl;

        ActiveListChange *active_list_change = new ActiveListChange();
        active_list_change->setAdded(node);
        send(active_list_change, "addedActivePeerOutput");
    } else {
        // if they requested check if we want to connect and send reply accordingly
        if (peer_list->getActiveListSize() < num_neighbors) {
            // accept
            if (peer_list->isPassive(node)) {
                peer_list->activatePeer(node);
            } else if (!peer_list->isActive(node)) {
                peer_list->addActivePeer(node);
            }

            int active_list_size = peer_list->getActiveListSize();
            emit(active_list_update_signal, active_list_size);
            EV_DEBUG << "accepted neighbor request from " << node << " (" << active_list_size
                << " peers)" << endl;

            ActiveListChange *active_list_change = new ActiveListChange();
            active_list_change->setAdded(node);
            send(active_list_change, "addedActivePeerOutput");

            Neighbor *reply = new Neighbor();
            reply->setReceiver(node);
            send(reply, "out");
        } else {
            EV_DEBUG << "rejected neighbor request by " << node << "("
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

void ActiveListManager::handleDisconnect(Disconnect *disconnect) {
    int peer_id = disconnect->getSender();
    if (peer_list->isActive(peer_id)) {
        EV_DEBUG << "received disconnect request from " << peer_id << ", passivating peer ("
            << peer_list->getActiveListSize() - 1 << " peers)" << endl;
        peer_list->passivatePeer(peer_id);
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
