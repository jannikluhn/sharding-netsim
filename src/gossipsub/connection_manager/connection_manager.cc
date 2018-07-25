#include <omnetpp.h>
#include <algorithm>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "connection_manager.h"

using namespace omnetpp;


Define_Module(ConnectionManager);


void ConnectionManager::initialize() {
    // load parameters
    c_rand = par("cRand").intValue();
    min_peers = par("minPeers").intValue();
    join_ttl = par("joinTTL").intValue();

    const char *contact_node_string = par("contactNodes").stringValue();
    contact_nodes = cStringTokenizer(contact_node_string).asIntVector();
    if (contact_nodes.size() == 0) {
        error("Empty contact node list");
    }

    state = State::WAITING_FOR_NODES;
    node_id = getParentModule()->getId();

    // initialize passive peer list to all contact nodes
    for (auto contact_node : contact_nodes) {
        addPassivePeer(contact_node);
    }

    // request nodes from random contact node
    GetNodes *get_nodes = new GetNodes();
    int contact_node_index = intuniform(0, contact_nodes.size() - 1);
    get_nodes->setReceiver(contact_nodes[contact_node_index]);
    send(get_nodes, "out");
}


//
// Message handlers
//
void ConnectionManager::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("getNodesInput")) {
        handleGetNodes(dynamic_cast<GetNodes *>(msg));
    } else if (msg->arrivedOn("nodesInput")) {
        handleNodes(dynamic_cast<Nodes *>(msg));
    } else if (msg->arrivedOn("joinInput")) {
        handleJoin(dynamic_cast<Join *>(msg));
    } else if (msg->arrivedOn("forwardJoinInput")) {
        handleForwardJoin(dynamic_cast<ForwardJoin *>(msg));
    } else if (msg->arrivedOn("nodesInput")) {
        handleNodes(dynamic_cast<Nodes *>(msg));
    } else if (msg->arrivedOn("getNodesInput")) {
        handleGetNodes(dynamic_cast<GetNodes *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }
    delete msg;
}

void ConnectionManager::handleGetNodes(GetNodes *msg) {
    // respond with list of all known peers
    Nodes *nodes = new Nodes();
    nodes->setPeersArraySize(active_list.size() + passive_list.size());
    int i = 0;
    for (auto active_peer_id : active_list) {
        nodes->setPeers(i, active_peer_id);
        i++;
    }
    for (auto passive_peer_id : passive_list) {
        nodes->setPeers(i, passive_peer_id);
        i++;
    }
    send(nodes, msg->getSender());
}

void ConnectionManager::handleNodes(Nodes *msg) {
    if (state != State::WAITING_FOR_NODES) {
        error("received NODES in state %s", state);
    }

    // add new nodes to passive list
    for (int i = 0; i < msg->getPeersArraySize(); i++) {
        addPassivePeer(msg->getPeers(i));
    }

    // send JOIN to random nodes from passive list
    int num_receivers = std::min<int>(passive_list.size(), c_rand);
    std::set<int> receivers;
    while (receivers.size() < num_receivers) {
        int list_index = intuniform(0, passive_list.size());
        receivers.insert(passive_list[list_index]);
    }
    for (auto receiver : receivers) {
        Join *join_msg = new Join();
        join_msg->setPeer(node_id);
        join_msg->setReceiver(receiver);
        join_msg->setTtl(join_ttl);
        send(join_msg, "out");
    }
}

void ConnectionManager::handleJoin(Join *msg) {
    int ttl = msg->getTtl();
    if (active_list.size() < min_peers || ttl == 0) {
        // add to active list
        int peer_id = msg->getSender();
        addActivePeer(peer_id);

        // send NEIGHBOR to new active peer
        Neighbor *neighbor_msg = new Neighbor();
        neighbor_msg->setPeer(node_id);
        neighbor_msg->setReceiver(peer_id);

        // send FORWARDJOIN
        ForwardJoin *forward_join_msg = new ForwardJoin();
        forward_join_msg->setPeer(peer_id);
        forward_join_msg->setTtl(forward_join_ttl);
        forward_join_msg->setReceiver(active_list[intuniform(0, active_list.size())]);
        send(forward_join_msg, "out");

        // TODO: notify new active peer listeners
    } else {
        // don't connect but forward join to random active peer
        Join *forwarded_msg = msg->dup();
        forwarded_msg->setTtl(ttl - 1);
        int receiver_index = intuniform(0, active_list.size());
        forwarded_msg->setReceiver(active_list[receiver_index]);
    }
}

void ConnectionManager::handleForwardJoin(ForwardJoin *msg) {
    int peer_id = msg->getPeer();
    addPassivePeer(peer_id);

    int ttl = msg->getTtl();
    if (ttl > 0) {
        ForwardJoin *forwarded_msg = msg->dup();
        forwarded_msg->setTtl(ttl - 1);
        forwarded_msg->setReceiver(active_list[intuniform(0, active_list.size())]);
        send(forwarded_msg, "out");
    }
}


//
// Helpers for adding and removing peers from active and passive lists
//
void ConnectionManager::addActivePeer(int node_id) {
    for (auto active_peer : active_list) {
        if (node_id == active_peer) {
            return;
        }
    }
    active_list.push_back(node_id);
}

void ConnectionManager::addPassivePeer(int node_id) {
    for (auto passive_peer : active_list) {
        if (node_id == passive_peer) {
            return;
        }
    }
    active_list.push_back(node_id);
}

void ConnectionManager::removeActivePeer(int node_id) {
    active_list.erase(
        std::remove(active_list.begin(), active_list.end(), node_id),
        active_list.end()
    );
}

void ConnectionManager::removePassivePeer(int node_id) {
    active_list.erase(
        std::remove(active_list.begin(), active_list.end(), node_id),
        active_list.end()
    );
}
