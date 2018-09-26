#include "kad_manager.h"
#include "../kademlia_packets_m.h"
#include <omnetpp.h>

using namespace omnetpp;


Define_Module(KadManager);


int KadManager::numInitStages() const {
    return 2;
}

void KadManager::initialize(int stage) {
    if (stage == 0) {
        const char *peer_table_path = par("peerTablePath").stringValue();
        peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));

        node_id = par("nodeId").intValue();
        hidden = par("hidden").boolValue();
        lookup_concurrency = par("lookupConcurrency").intValue();
        max_lookup_round_duration = par("maxLookupRoundDuration").doubleValue();
        lookup_interval = par("lookupInterval").doubleValue();

        lookup_ongoing = false;
    } else if (stage == 1) {
        int r = intuniform(0, std::numeric_limits<int>::max());
        KadId home_id = KadId(r);
        EV_DEBUG << "setting home id to SHA256(" << r << ") or " << home_id << endl;
        peer_table->setHomeId(home_id);

        self_msg = new cMessage();
        scheduleAt(simTime() + uniform(0, lookup_interval), self_msg);

        // ping bootnodes who should reply with ADD_ME
        for (int i = 0; i < par("numBootnodes").intValue(); i++) {
            if (node_id == i) {
                continue;
            } else {
                EV_DEBUG << "pinging bootnode " << i << endl;
                KadPing *ping = new KadPing();
                ping->setReceiver(i);
                ping->setSenderKadId(home_id);
                send(ping, "out");

                if (!hidden) {
                    KadAddMe *add_me = new KadAddMe();
                    add_me->setSenderKadId(home_id);
                    add_me->setReceiver(i);
                    send(add_me, "out");
                }
            }
        }
    }
}

void KadManager::finish() {
    delete self_msg;
}

void KadManager::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSelf(msg);
    } else if (msg->arrivedOn("neighborsInput")) {
        handleNeighbors(check_and_cast<KadNeighbors *>(msg));
    } else if (msg->arrivedOn("pongInput")) {
        handlePong(check_and_cast<KadPong *>(msg));
    } else if (msg->arrivedOn("addMeInput")) {
        handleAddMe(check_and_cast<KadAddMe *>(msg));
    } else {
        error("unhandled message");
    }
}

void KadManager::handleSelf(cMessage *self) {
    if (lookup_ongoing) {
        startNextLookupRound();
    } else if (peer_table->size() == 0) {
        EV_WARN << "no peers, skipping lookup" << endl;
        scheduleAt(simTime() + lookup_interval, self_msg);
    } else {
        EV_DEBUG << "looking up random node" << endl;
        int r = intuniform(0, std::numeric_limits<int>::max());
        KadId kad_id = KadId(r);
        lookup(kad_id);
    }
}

void KadManager::handleNeighbors(KadNeighbors *neighbors) {
    KadId sender_kad_id = neighbors->getSenderKadId();

    bool expected = pending_neighbors.count(sender_kad_id) > 0;
    if (!expected) {
        EV_WARN << "received unexpected NEIGHBORS from " << sender_kad_id << endl;
    }
    pending_neighbors.erase(sender_kad_id);

    // add neighbors to peer table
    int known = 0;
    int unknown = 0;
    KadId home_id = peer_table->getHomeId();
    for (int i = 0; i < neighbors->getKadIdsArraySize(); i++) {
        KadId kad_id = neighbors->getKadIds(i);
        int node_id = neighbors->getNodeIds(i);

        if (kad_id == home_id) {
            continue;
        }

        if (peer_table->contains(kad_id)) {
            known += 1;
            peer_table->update(kad_id);
        } else {
            unknown += 1;

            // check that it's alive
            KadPing *ping = new KadPing();
            ping->setSenderKadId(home_id);
            ping->setReceiver(node_id);
            send(ping, "out");

            // ask to be added into their routing table
            if (!hidden) {
                KadAddMe *add_me = new KadAddMe();
                add_me->setSenderKadId(home_id);
                add_me->setReceiver(node_id);
                send(add_me, "out");
            }

            // in case of lookup, wait for PONG and ADD_ME before sending FIND_NODE
            if (expected) {
                pending_pongs.insert(kad_id);
                pending_add_mes.insert(kad_id);
            }
        }
    }
    EV_DEBUG << "received NEIGHBORS from " << sender_kad_id << " with " << known
        << " known and " << unknown << " new nodes" << endl;

    delete neighbors;
}

void KadManager::handlePong(KadPong *pong) {
    KadId kad_id = pong->getSenderKadId();
    peer_table->updateIfKnown(kad_id);

    if (pending_pongs.count(kad_id) > 0) {
        EV_DEBUG << "received expected PONG from " << kad_id << endl;
        if (queried.count(kad_id) == 0 && peer_table->contains(kad_id)) {
            candidates.insert(kad_id);
        }
        pending_pongs.erase(kad_id);
    } else {
        EV_DEBUG << "received unexpected PONG from " << kad_id << endl;
    }

    if (lookup_ongoing && pending_neighbors.size() == 0 && pending_pongs.size() == 0 &&
            pending_add_mes.size() == 0) {
        startNextLookupRound();
    }

    delete pong;
}

void KadManager::handleAddMe(KadAddMe *add_me) {
    KadId kad_id = add_me->getSenderKadId();
    int node_id = add_me->getSender();

    if (pending_add_mes.count(kad_id) > 0) {
        EV_DEBUG << "received expected ADD_ME from " << kad_id << endl;
        if (queried.count(kad_id) == 0 && peer_table->contains(kad_id)) {
            candidates.insert(kad_id);
        }
        pending_add_mes.erase(kad_id);
    } else {
        EV_DEBUG << "received unexpected ADD_ME from " << kad_id << endl;
    }

    if (peer_table->contains(kad_id)) {
        EV_DEBUG << kad_id << " is already known" << endl;
    } else if (peer_table->insertPossible(kad_id)) {
        EV_DEBUG << "Inserting " << kad_id << " with address " << node_id << " into peer table"
            << endl;
        peer_table->insert(kad_id, node_id);
    } else {
        // TODO: check if some node is offline
        EV_DEBUG << "Ignoring ADD_ME from " << kad_id << " as bucket is full"
            << endl;
    }

    if (lookup_ongoing && pending_neighbors.size() == 0 && pending_pongs.size() == 0 &&
            pending_add_mes.size() == 0) {
        startNextLookupRound();
    }

    delete add_me;
}

void KadManager::startNextLookupRound() {
    if (!lookup_ongoing) {
        error("No lookup ongoing");
    }
    if (self_msg->isScheduled()) {
        cancelEvent(self_msg);
    }

    // return if that was the last round
    if (is_last_lookup_round) {
        EV_DEBUG << "lookup finished, got " << peer_table->size() << " peers now" << endl;
        lookup_ongoing = false;
        scheduleAt(simTime() + lookup_interval, self_msg);
        return;
    }
    EV_DEBUG << "starting next lookup round (pending neighbors: " << pending_neighbors.size()
        << ", pending pongs: " << pending_pongs.size() << ", pending add_mes: "
        << pending_add_mes.size() << ")" << endl;

    // ignore responses that haven't been received in time
    pending_neighbors.clear();
    pending_pongs.clear();
    pending_add_mes.clear();

    // if the last round has made progress, ask LOOKUP_CONCURRENCY peers, otherwise ask
    // BUCKET_SIZE peers
    std::vector<KadId> receivers;

    bool to_finish;
    if (candidates.size() == 0) {
        to_finish = true;
    } else if (is_first_lookup_round) {
        to_finish = false;
        last_closest_candidate = lookup_target.getNeighbors(candidates, 1)[0];
    } else {
        KadId closest_candidate = lookup_target.getNeighbors(candidates, 1)[0];
        to_finish = !lookup_target.isCloser(closest_candidate, last_closest_candidate);
        last_closest_candidate = closest_candidate;
    }

    if (to_finish) {
        EV_DEBUG << "last lookup round made no progress, this will be the final one" << endl;

        is_last_lookup_round = true;
        receivers = lookup_target.getNeighbors(candidates, BUCKET_SIZE);
    } else {
        receivers = lookup_target.getNeighbors(candidates, lookup_concurrency);
    }

    for (auto receiver : receivers) {
        candidates.erase(receiver);
        queried.insert(receiver);
        pending_neighbors.insert(receiver);

        EV_DEBUG << "sending FIND_NODE to " << receiver << endl;
        KadFindNode *find_node = new KadFindNode();
        find_node->setSenderKadId(peer_table->getHomeId());
        find_node->setReceiver(peer_table->getNodeId(receiver));
        find_node->setTargetKadId(lookup_target);
        send(find_node, "out");
    }

    is_first_lookup_round = false;
    // set timeout for next lookup round
    lookup_round_end_time = simTime() + max_lookup_round_duration;
    scheduleAt(lookup_round_end_time, self_msg);
}

void KadManager::lookup(KadId kad_id) {
    EV_DEBUG << "looking up " << kad_id << endl;
    if (lookup_ongoing) {
        error("lookup already ongoing");
    }

    lookup_ongoing = true;
    lookup_target = kad_id;
    is_first_lookup_round = true;
    is_last_lookup_round = false;

    queried.clear();
    pending_neighbors.clear();
    pending_pongs.clear();
    pending_add_mes.clear();
    candidates.clear();

    for (auto candidate : peer_table->getClosestPeers(kad_id, lookup_concurrency)) {
        candidates.insert(candidate);
    }
    if (candidates.size() == 0) {
        error("no peers");
    }

    startNextLookupRound();
}
