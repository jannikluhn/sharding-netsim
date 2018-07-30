#include <omnetpp.h>
#include "joiner.h"
#include "../../packets_m.h"

using namespace omnetpp;


Define_Module(Joiner);


void Joiner::initialize() {
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
}

void Joiner::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("joinInput")) {
        handleJoin(check_and_cast<Join *>(msg));
    } else if (msg->arrivedOn("neighborInput")) {
        handleNeighbor(check_and_cast<Neighbor *>(msg));
    } else if (msg->arrivedOn("disconnectInput")) {
        handleDisconnect(check_and_cast<Disconnect *>(msg));
    } else if (msg->arrivedOn("forwardJoinInput")) {
        handleForwardJoin(check_and_cast<ForwardJoin *>(msg));
    } else if (msg->arrivedOn("startInput")) {
        sendInitialJoins();
        delete msg;
    }
}

void Joiner::sendInitialJoins() {
    int num_receivers = std::max(num_random_neighbors, peer_list->getPassiveListSize());
    if (num_receivers == 0) {
        error("No contact nodes to join with");
    }

    EV_INFO << "sending initial joins to " << num_receivers << " nodes\n";

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
}

void Joiner::startHeartbeat() {
    scheduleAt(simTime() + uniform(0, heartbeat_interval), new cMessage());
}

void Joiner::handleHeartbeat(cMessage *heartbeat) {
    scheduleAt(simTime() + heartbeat_interval, heartbeat);

    if (peer_list->getActiveListSize() > num_neighbors) {
        EV_DEBUG << "Too many active peers, passivating random one\n";
        // disconnect from random peer
        int peer = peer_list->getRandomActivePeer();
        Disconnect *disconnect = new Disconnect();
        disconnect->setReceiver(peer);
        send(disconnect, "out");

        peer_list->passivatePeer(peer);
    } else if (peer_list->getActiveListSize() < num_neighbors && neighbor_requests.size() == 0) {
        EV_DEBUG << "Too few active peers, activating random one\n";
        // connect to random passive peer
        int peer = peer_list->getRandomPassivePeer();
        Neighbor *neighbor = new Neighbor();
        neighbor->setReceiver(peer);
        send(neighbor, "out");

        neighbor_requests.insert(peer);
    }
}

void Joiner::handleJoin(Join *join) {
    int ttl = join->getTtl();
    int node = join->getNode();

    if (ttl > 0 && peer_list->getActiveListSize() >= num_neighbors) {
        EV_DEBUG << "forwarding received join\n";

        // forward join to random active peer
        join->setTtl(ttl - 1);
        do {
            join->setReceiver(peer_list->getRandomActivePeer());
        } while (join->getReceiver() == join->getSender());

        send(join, "out");
    } else if (peer_list->isActive(node)) {
        EV_DEBUG << "received join from already active peer, forwarding\n";

        // forward join to random active peer, without decrementing TTL
        do {
            join->setReceiver(peer_list->getRandomActivePeer());
        } while (join->getReceiver() == join->getSender());

        send(join, "out");
    } else {
        EV_DEBUG << "accepting join\n";

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

void Joiner::handleNeighbor(Neighbor *neighbor) {
    // TODO: accept if peer has too few neighbors and if it is "near"
    int node = neighbor->getSender();

    if (neighbor_requests.count(node) > 0) {
        EV_DEBUG << "received positive response to neighbor request\n";

        // if we requested to connect just add the peer if we haven't already
        neighbor_requests.erase(node);
        if (peer_list->isPassive(node)) {
            peer_list->activatePeer(node);
        } else if (!peer_list->isActive(node)) {
            peer_list->addActivePeer(node);
        }
    } else {
        // if they requested check if we want to connect and send reply accordingly
        if (peer_list->getActiveListSize() < num_neighbors) {
            EV_DEBUG << "accepting neighbor request\n";
            // accept
            if (peer_list->isPassive(node)) {
                peer_list->activatePeer(node);
            } else if (!peer_list->isActive(node)) {
                peer_list->addActivePeer(node);
            }

            Neighbor *reply = new Neighbor();
            reply->setReceiver(node);
            send(reply, "out");
        } else {
            EV_DEBUG << "rejecting neighbor request\n";
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

void Joiner::handleDisconnect(Disconnect *disconnect) {
    EV_DEBUG << "received disconnect request, removing peer\n";
    neighbor_requests.erase(disconnect->getSender());
    delete disconnect;
}

void Joiner::handleForwardJoin(ForwardJoin *forward_join) {
    int node = forward_join->getNode();
    int ttl = forward_join->getTtl();

    // add node to passive list
    if (!peer_list->isPeer(node)) {
        EV_DEBUG << "adding passive peer from FORWARDJOIN\n";
        peer_list->addPassivePeer(node);
    }

    // forward to random active peer if not dead
    if (ttl > 0) {
        forward_join->setTtl(ttl - 1);
        forward_join->setReceiver(peer_list->getRandomActivePeer());
        send(forward_join, "out");
    } else {
        delete forward_join;
    }
}
