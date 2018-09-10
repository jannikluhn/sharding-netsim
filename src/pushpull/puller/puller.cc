#include <omnetpp.h>
#include "../pushpull_packets_m.h"
#include "puller.h"
#include <algorithm>

using namespace omnetpp;


Define_Module(Puller);


void Puller::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    push_fanout = par("pushFanout").intValue();
    pull_fanout = par("pullFanout").intValue();
    start_time = par("startTime").doubleValue();
    period = par("period").doubleValue();
    request_interval = par("requestInterval").doubleValue();
    push_time_delta = par("pushTimeDelta").doubleValue();

    push_time = period / 20;

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
        EV_DEBUG << "requesting content with id " << content_id << endl;
        EV_DEBUG << "increasing push time to " << push_time << endl;
    } else {
        push_time = push_time * (1 - push_time_delta);
        EV_DEBUG << "decreasing push time to " << push_time << endl;
    }

    scheduleAt(getEmissionTime(content_id + 1) + push_time, heartbeat);
}

void Puller::handleRequestTrigger(cMessage *request_trigger) {
    int content_id = request_triggers[request_trigger];
    if (cache->contains(content_id)) {
        EV_DEBUG << "pulling content with id " << content_id << " was successful" << endl;
        request_triggers.erase(request_trigger);
        next_request_time.erase(content_id);
        delete request_trigger;
    } else {
        EV_DEBUG << "requesting content with id " << content_id << " again" << endl;
        request(content_id);
        scheduleAt(simTime() + request_interval, request_trigger);
    }
}

void Puller::request(int content_id) {
    Pull *pull = new Pull();
    pull->setContentIdsArraySize(1);
    pull->setContentIds(0, content_id);

    int i = 0;
    for (auto peer : pull_peers) {
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

    for (auto peer : push_peers) {
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
            for (auto peer : push_peers) {
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
        idle_peers.push_back(peer_list_change->getAddedPeers(i));
    }

    for (int i = 0; i < peer_list_change->getRemovedPeersArraySize(); i++) {
        push_peers.erase(
            std::remove(push_peers.begin(), push_peers.end(), peer_list_change->getRemovedPeers(i)),
            push_peers.end()
        );
        pull_peers.erase(
            std::remove(pull_peers.begin(), pull_peers.end(), peer_list_change->getRemovedPeers(i)),
            pull_peers.end()
        );
        idle_peers.erase(
            std::remove(idle_peers.begin(), idle_peers.end(), peer_list_change->getRemovedPeers(i)),
            idle_peers.end()
        );
    }

    while (push_peers.size() < push_fanout && idle_peers.size() > 0) {
        push_peers.push_back(idle_peers.front());
        idle_peers.pop_front();
    }
    while (pull_peers.size() < pull_fanout && idle_peers.size() > 0) {
        pull_peers.push_back(idle_peers.front());
        idle_peers.pop_front();
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
