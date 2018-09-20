#include <omnetpp.h>
#include "../kademlia_packets_m.h"
#include "kad_manager.h"

using namespace omnetpp;


Define_Module(KadManager);


void KadManager::initialize() {
    const char *peer_table_path = par("peerTablePath").stringValue();
    peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));

    node_id = par("nodeId").intValue();
    shard_id = par("shardId").intValue();
    max_peers = par("maxPeers").intValue();
    hidden = par("hidden").boolValue();
    lookup_concurrency = par("lookupConcurrency").intValue();
    max_lookup_round_duration = par("maxLookupRoundDuration").doubleValue();

    lookup_ongoing = false;

    self_msg = new cMessage();
    scheduleAt(simTime(), self_msg);

    for (int i = 0; i < par("numBootnodes").intValue(); i++) {
        if (shard_id == 0 && node_id == i) {
            continue;
        } else {
            KadId kad_id = {i, 0};
            peer_table->insert(kad_id);

            if (!hidden) {
                KadAddMe *add_me = new KadAddMe();
                add_me->setShard(kad_id.shard_id);
                add_me->setReceiver(kad_id.node_id);
                send(add_me, "out");
            }
        }
    }
}

void KadManager::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSelf(msg);
    } else if (msg->arrivedOn("neighborsInput")) {
        handleNeighbors(check_and_cast<KadNeighbors *>(msg));
    } else if (msg->arrivedOn("pongInput")) {
        handlePong(check_and_cast<KadPong *>(msg));
    } else {
        error("unhandled message");
    }
}

void KadManager::handleSelf(cMessage *self) {
    if (lookup_ongoing) {
        startNextLookupRound();
    } else if (peer_table->size() < max_peers && !lookup_ongoing) {
        EV_DEBUG << "not enough peers, looking up random node" << endl;
        // exact numbers don't really matter, as they are hashed anyway
        KadId kad_id = {intuniform(0, 10000), intuniform(0, 1023)};  
        lookup(kad_id);
    }
}

void KadManager::handleNeighbors(KadNeighbors *neighbors) {
    int sender = neighbors->getSender();
    int shard = neighbors->getShard();
    KadId kad_id = {sender, shard};

    bool expected = pending_neighbors.count(kad_id) > 0;
    if (!expected) {
        EV_WARN << "received unexpected NEIGHBORS from " << shard << "/" << sender << endl;
    }
    pending_neighbors.erase(kad_id);

    int known = 0;
    int unknown = 0;
    for (int i = 0; i < neighbors->getNodesArraySize(); i++) {
        int neighbor_node = neighbors->getNodes(i);
        int neighbor_shard = neighbors->getShards(i);
        KadId neighbor_kad_id = {neighbor_node, neighbor_shard};

        if (neighbor_node == node_id && neighbor_shard == shard_id) {
            continue;
        }

        if (peer_table->contains(neighbor_kad_id)) {
            known += 1;
            peer_table->update(neighbor_kad_id);
        } else {
            unknown += 1;

            // check that it's alive
            KadPing *ping = new KadPing();
            ping->setShard(shard_id);
            ping->setReceiver(neighbor_node);
            send(ping, "out");

            // ask to be added into their routing table
            if (!hidden) {
                KadAddMe *add_me = new KadAddMe();
                add_me->setShard(shard_id);
                add_me->setReceiver(neighbor_node);
                send(add_me, "out");
            }

            // in case of lookup, wait for pong before sending find node
            if (expected) {
                pending_pongs.insert(neighbor_kad_id);
            }
        }
    }
    EV_DEBUG << "received NEIGHBORS from " << shard << "/" << sender << " with " << known
        << " known and " << unknown << " new nodes" << endl;

    delete neighbors;
}

void KadManager::handlePong(KadPong *pong) {
    int sender = pong->getSender();
    int shard = pong->getShard();

    KadId kad_id = {sender, shard};
    peer_table->updateIfKnown(kad_id);

    if (pending_pongs.count(kad_id) > 0) {
        EV_DEBUG << "received expected PONG from " << shard << "/" << sender << endl;
        if (queried.count(kad_id) == 0) {
            candidates.insert(kad_id);
        }
        pending_pongs.erase(kad_id);
    } else {
        EV_DEBUG << "received unexpected PONG from " << shard << "/" << sender << endl;
    }

    if (lookup_ongoing && pending_neighbors.size() == 0 && pending_pongs.size() == 0) {
        startNextLookupRound();
    }

    delete pong;
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
        scheduleAt(simTime() + 5, self_msg);
        return;
    }
    EV_DEBUG << "starting next lookup round (pending neighbors: " << pending_neighbors.size()
        << ", pending pongs: " << pending_pongs.size() << ")" << endl;

    // ignore responses that haven't been received in time
    pending_neighbors.clear();
    pending_pongs.clear();

    // if the last round has made progress, ask LOOKUP_CONCURRENCY peers, otherwise ask
    // BUCKET_SIZE peers
    std::vector<KadId> receivers;

    bool to_finish;
    if (candidates.size() == 0) {
        to_finish = true;
    } else {
        KadId closest_candidate = lookup_target.getNeighbors(candidates, 1)[0];
        to_finish = (
            !is_first_lookup_round &&
            !lookup_target.isCloser(closest_candidate, last_closest_candidate)
        );
        last_closest_candidate = closest_candidate;
    }

    if (to_finish) {
        EV_DEBUG << "last lookup round made no progress, this will be the last one" << endl;
        
        is_last_lookup_round = true;
        receivers = lookup_target.getNeighbors(candidates, BUCKET_SIZE);
    } else {
        receivers = lookup_target.getNeighbors(candidates, lookup_concurrency);
    }

    for (auto receiver : receivers) {
        candidates.erase(receiver);
        queried.insert(receiver);
        pending_neighbors.insert(receiver);

        EV_DEBUG << "sending FIND_NODE to " << receiver.shard_id << "/" << receiver.node_id
            << endl;
        KadFindNode *find_node = new KadFindNode();
        find_node->setShard(shard_id);
        find_node->setReceiver(receiver.node_id);
        find_node->setTargetNode(lookup_target.node_id);
        find_node->setTargetShard(lookup_target.shard_id);
        send(find_node, "out");
    }

    is_first_lookup_round = false;
    // set timeout for next lookup round
    lookup_round_end_time = simTime() + max_lookup_round_duration;
    scheduleAt(lookup_round_end_time, self_msg);
}

void KadManager::lookup(KadId kad_id) {
    EV_DEBUG << "looking up " << kad_id.shard_id << "/" << kad_id.node_id << endl;
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
    candidates.clear();

    for (auto candidate : peer_table->getNeighborsInBuckets(kad_id, lookup_concurrency)) {
        candidates.insert(candidate);
    }
    if (candidates.size() == 0) {
        error("no peers");
    }

    startNextLookupRound();
}
