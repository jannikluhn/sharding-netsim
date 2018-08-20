#include <omnetpp.h>
#include <algorithm>
#include "../internal_messages_m.h"
#include "passive_list_manager.h"

using namespace omnetpp;


Define_Module(PassiveListManager);


void PassiveListManager::initialize() {
    view_initialization_finished = false;
    node_id = par("nodeId").intValue();

    shuffle_interval = par("shuffleInterval").doubleValue();
    active_shuffling_size = par("activeShufflingSize").intValue();
    passive_shuffling_size = par("passiveShufflingSize").intValue();
    shuffle_ttl = par("shuffleTTL").intValue();
    passive_list_size = par("passiveListSize").intValue();

    num_pending_shuffle_requests = 0;

    peer_list_update_signal = registerSignal("peerListUpdate");

    const char *peer_list_path = par("peerListPath").stringValue();
    peer_list = check_and_cast<PeerList *>(getModuleByPath(peer_list_path));

    cMessage *heartbeat = new cMessage();
    scheduleAt(simTime() + uniform(0, shuffle_interval), heartbeat);

    startViewInitialization();
}

void PassiveListManager::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("nodesInput")) {
        handleNodes(check_and_cast<Nodes *>(msg));
    } else if (msg->arrivedOn("shuffleInput")) {
        handleShuffle(check_and_cast<Shuffle *>(msg));
    } else if (msg->arrivedOn("shuffleReplyInput")) {
        handleShuffleReply(check_and_cast<ShuffleReply *>(msg));
    } else if (msg->arrivedOn("forwardJoinInput")) {
        handleForwardJoin(check_and_cast<ForwardJoin *>(msg));
    } else {
        error("unhandled message");
    }
}


void PassiveListManager::handleHeartbeat(cMessage *heartbeat) {
    initiateShuffle();

    scheduleAt(simTime() + shuffle_interval, heartbeat);
}


//
// View initialization
//
void PassiveListManager::startViewInitialization() {
    emit(peer_list_update_signal, peer_list->getPeerListSize());

    // send GETNODES to all passive peers (which at the moment should be just the contact nodes)
    for (int i = 0; i < peer_list->getPassiveListSize(); i++) {
        int receiver = peer_list->getPassivePeerByIndex(i);

        GetNodes *get_nodes = new GetNodes();
        get_nodes->setReceiver(receiver);
        send(get_nodes, "out");

        pending_getnodes_requests.insert(receiver);
    }
}

void PassiveListManager::handleNodes(Nodes *nodes) {
    if (view_initialization_finished) {
        error("received unrequested NODES");
    }

    pending_getnodes_requests.erase(nodes->getSender());


    int initial_peer_list_size = peer_list->getPeerListSize();
    for (int i = 0; i < nodes->getPeersArraySize(); i++) {
        int peer_id = nodes->getPeers(i);
        if (peer_id != node_id && !peer_list->isPeer(peer_id)) {
            peer_list->addPassivePeer(nodes->getPeers(i));
        }
    }
    int final_peer_list_size = peer_list->getPeerListSize();
    if (final_peer_list_size != initial_peer_list_size) {
        emit(peer_list_update_signal, final_peer_list_size);
    }

    if (pending_getnodes_requests.empty()) {
        EV_DEBUG << "View initialization complete. Passive list size: "
            << peer_list->getPassiveListSize();

        view_initialization_finished = true;

        int gate_base_id = gateBaseId("viewInitializationFinishedOutputs");
        for (int i = 0; i < gateSize("viewInitializationFinishedOutputs"); i++) {
            send(new ViewInitializationFinished(), gate_base_id + i);
        }
    }

    delete nodes;
}


//
// Shuffling
//
void PassiveListManager::initiateShuffle() {
    // send SHUFFLE to random active peer with some of our active and passive peers

    if (peer_list->getActiveListSize() == 0) {
        return;
    }

    int receiver = peer_list->getRandomActivePeer();
    EV_DEBUG << "initiating passive list shuffle with " << receiver << endl;

    Shuffle *shuffle = new Shuffle();
    shuffle->setReceiver(receiver);
    shuffle->setNode(node_id);
    shuffle->setTtl(shuffle_ttl);

    int num_active_peers = peer_list->getActiveListSize();
    int num_active = std::min(num_active_peers, active_shuffling_size);
    std::vector<int> active_shuffling = peer_list->getActiveListShuffling();

    int num_passive_peers = peer_list->getPassiveListSize();
    int num_passive = std::min(num_passive_peers, passive_shuffling_size);
    std::vector<int> passive_shuffling = peer_list->getPassiveListShuffling();

    shuffle->setPeersArraySize(num_active + num_passive);
    for (int i = 0; i < num_active; i++) {
        shuffle->setPeers(i, active_shuffling[i]);
    }
    for (int i = 0; i < num_passive; i++) {
        shuffle->setPeers(num_active + i, passive_shuffling[i]);
    }

    send(shuffle, "out");
    num_pending_shuffle_requests++;
}

void PassiveListManager::handleShuffle(Shuffle *shuffle) {
    int ttl = shuffle->getTtl();
    if (ttl > 0 && peer_list->getActiveListSize() > 1) {
        // forward SHUFFLE to random active peer
        EV_DEBUG << "forwarding alive SHUFFLE" << endl;
        shuffle->setTtl(ttl - 1);
        do {
            shuffle->setReceiver(peer_list->getRandomActivePeer());
        } while (shuffle->getReceiver() == shuffle->getSender());
        send(shuffle, "out");
    } else {
        int node = shuffle->getNode();
        EV_DEBUG << "responding to dead shuffle from " << node << endl;

        // send SHUFFLEREPLY
        ShuffleReply *shuffle_reply = new ShuffleReply();
        shuffle_reply->setReceiver(node);

        int num_peers = std::min(
            peer_list->getPassiveListSize(),
            static_cast<int>(shuffle->getPeersArraySize())
        );
        std::vector<int> peers = peer_list->getPassiveListShuffling();
        shuffle_reply->setPeersArraySize(num_peers);
        for (int i = 0; i < num_peers; i++) {
            shuffle_reply->setPeers(i, peers[i]);
        }
        send(shuffle_reply, "out");

        // add peers to passive list
        int initial_peer_list_size = peer_list->getPeerListSize();
        for (int i = 0; i < shuffle->getPeersArraySize(); i++) {
            int peer = shuffle->getPeers(i);
            if (peer_list->isPeer(peer) || peer == node_id) {
                continue;
            }

            if (peer_list->getPassiveListSize() > passive_list_size) {
                peer_list->dropRandomPassivePeer();
            }
            peer_list->addPassivePeer(peer);
        }
        int final_peer_list_size = peer_list->getPeerListSize();
        if (final_peer_list_size != initial_peer_list_size) {
            emit(peer_list_update_signal, final_peer_list_size);
        }

        delete shuffle;
    }
}

void PassiveListManager::handleShuffleReply(ShuffleReply *shuffle_reply) {
    if (num_pending_shuffle_requests == 0) {
        error("received unexpected SHUFFLE");
    }
    num_pending_shuffle_requests--;
    int initial_peer_list_size = peer_list->getPeerListSize();

    EV_DEBUG << "received shuffle reply from " << shuffle_reply->getSender() << endl;

    // add peers to passive list
    for (int i = 0; i < shuffle_reply->getPeersArraySize(); i++) {
        int peer = shuffle_reply->getPeers(i);
        if (peer_list->isPeer(peer) || peer == node_id) {
            continue;
        }

        if (peer_list->getPassiveListSize() > passive_list_size) {
            peer_list->dropRandomPassivePeer();
        }
        peer_list->addPassivePeer(peer);
    }

    int final_peer_list_size = peer_list->getPeerListSize();
    if (final_peer_list_size != initial_peer_list_size) {
        emit(peer_list_update_signal, final_peer_list_size);
    }
    delete shuffle_reply;
}

void PassiveListManager::handleForwardJoin(ForwardJoin *forward_join) {
    int node = forward_join->getNode();
    int ttl = forward_join->getTtl();

    // add node to passive list
    if (!peer_list->isPeer(node)) {
        EV_DEBUG << "adding passive peer from FORWARDJOIN" << endl;
        peer_list->addPassivePeer(node);
        emit(peer_list_update_signal, peer_list->getPeerListSize());
    }

    // forward to random active peer if not dead
    if (ttl > 0 && peer_list->getActiveListSize() > 0) {
        forward_join->setTtl(ttl - 1);
        forward_join->setReceiver(peer_list->getRandomActivePeer());
        send(forward_join, "out");
    } else {
        delete forward_join;
    }
}
