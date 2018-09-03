#include <omnetpp.h>
#include "../pushpull_packets_m.h"
#include "puller.h"
#include <algorithm>

using namespace omnetpp;


Define_Module(Puller);


void Puller::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    pull_fanout = par("pullFanout").intValue();
    start_time = par("startTime").doubleValue();
    period = par("period").doubleValue();
    request_interval = par("requestInterval").doubleValue();
    push_time_delta = par("pushTimeDelta").doubleValue();

    push_time = period / 10;

    gapless_synced_until = -1;

    new_gossip_received_signal = registerSignal("newGossipReceived");

    cMessage *heartbeat = new cMessage();
    scheduleAt(start_time + push_time, heartbeat);
}

void Puller::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (request_triggers.count(msg) > 0) {
            handleRequestTrigger(msg);
        } else {
            handleHeartbeat(msg);
        }
    } else if (msg->arrivedOn("sourceInput")) {
        handleSourceGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("gossipInput")) {
        handleExternalGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("peerListChangeInput")) {
        handlePeerListChange(check_and_cast<PeerListChange *>(msg));
    } else {
        error("unhandled message");
    }
}

void Puller::handleHeartbeat(cMessage *heartbeat) {
    int content_id = getCurrentContentId();
    if (!cache->contains(content_id)) {
        request(content_id);

        cMessage *request_trigger = new cMessage();
        scheduleAt(simTime() + request_interval, request_trigger);
        request_triggers[request_trigger] = content_id;

        push_time = push_time * (1 + push_time_delta);
    } else {
        push_time = push_time * (1 - push_time_delta);
    }

    scheduleAt(getEmissionTime(content_id + 1) + push_time, heartbeat);
}

void Puller::handleRequestTrigger(cMessage *request_trigger) {
    int content_id = request_triggers[request_trigger];
    if (cache->contains(content_id)) {
        request_triggers.erase(request_trigger);
        next_request_time.erase(content_id);
        delete request_trigger;
    } else {
        request(content_id);
        scheduleAt(simTime() + request_interval, request_trigger);
    }
}

void Puller::request(int content_id) {
    Pull *pull = new Pull();
    pull->setContentIdsArraySize(1);
    pull->setContentIds(0, content_id);

    int i = 0;
    for (auto peer : getPeerShuffling()) {
        Pull *dup = pull->dup();
        dup->setReceiver(peer);
        send(dup, "out");

        i++;
        if (i >= pull_fanout) {
            break;
        }
    }
    delete pull;

    next_request_time[content_id] = simTime() + request_interval;
}

void Puller::handleSourceGossip(Gossip *gossip) {
    int content_id = gossip->getContentId();
    if (cache->contains(content_id)) {
        error("Source created gossip with known id");
    }
    if (simTime() > getEmissionTime(content_id) + push_time) {
        error("not pushing my own gossip");
    }

    emit(new_gossip_received_signal, simTime() - gossip->getCreationTime());
    insertContentId(content_id, gossip->getCreationTime());

    for (auto peer : peers) {
        Gossip *forwarded_gossip = gossip->dup();
        forwarded_gossip->setReceiver(peer);
        send(forwarded_gossip, "out");
    }

    delete gossip;
}

void Puller::handleExternalGossip(Gossip *gossip) {
    int sender = gossip->getSender();
    int content_id = gossip->getContentId();

    if (!cache->contains(content_id)) {
        EV_DEBUG << "received new gossip with id " << content_id << " from " << sender << endl;
        insertContentId(content_id, gossip->getCreationTime());
        emit(new_gossip_received_signal, simTime() - gossip->getCreationTime());
        if (simTime() < getEmissionTime(content_id) + push_time) {
            EV_DEBUG << "forwarding it" << endl;
            for (auto peer : peers) {
                if (peer != sender) {
                    Gossip *dup = gossip->dup();
                    dup->setReceiver(peer);
                    send(dup, "out");
                }
            }
        } else {
            EV_DEBUG << "keeping it" << endl;
        }
    } else {
        EV_DEBUG << "received known gossip with id " << content_id << " from " << sender << endl;
    }

    delete gossip;
}

void Puller::handlePeerListChange(PeerListChange *peer_list_change) {
    for (int i = 0; i < peer_list_change->getAddedPeersArraySize(); i++) {
        peers.push_back(peer_list_change->getAddedPeers(i));
    }

    for (int i = 0; i < peer_list_change->getRemovedPeersArraySize(); i++) {
        peers.erase(
            std::remove(peers.begin(), peers.end(), peer_list_change->getRemovedPeers(i)),
            peers.end()
        );
    }

    delete peer_list_change;
}

void Puller::insertContentId(int new_content_id, simtime_t creation_time) {
    cache->insert(new_content_id, creation_time);
    for (int content_id = gapless_synced_until + 1; true; content_id++) {
        if (cache->contains(content_id)) {
            gapless_synced_until++;
        } else {
            break;
        }
    }
}

simtime_t Puller::getEmissionTime(int content_id) {
    return start_time + content_id * period;
}

int Puller::getCurrentContentId() {
    return (simTime() - start_time).dbl() / period;
}

std::vector<int> Puller::shuffle(std::vector<int> v) {
    // shuffle manually so that omnet++'s RNGs are used (see
    // https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm)
    for (int i = v.size() - 1; i >= 1; i--) {
        int j = intuniform(0, i);
        int value = v[j];
        v[j] = v[i];
        v[i] = value;
    }
    return v;
}

std::vector<int> Puller::getPeerShuffling() {
    return shuffle(peers);
}
