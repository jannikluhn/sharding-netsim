#include <omnetpp.h>
#include "../../utils/cache/cache.h"
#include "../peer_tracker/peer_tracker.h"
#include "gossip_handler.h"

using namespace omnetpp;


Define_Module(GossipHandler);


void GossipHandler::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    const char *peer_tracker_path = par("peerTrackerPath").stringValue();
    peer_tracker = check_and_cast<PeerTracker *>(getModuleByPath(peer_tracker_path));

    notification_interval = par("notificationInterval").doubleValue();
    min_eager_time = par("minEagerTime").doubleValue();

    new_gossip_received_signal = registerSignal("newGossipReceived");


    cMessage *scheduler_msg = new cMessage();
    scheduleAt(simTime() + notification_interval, scheduler_msg);
}


void GossipHandler::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleScheduler(msg);
    } else if (msg->arrivedOn("externalGossipInput")) {
        handleExternalGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("sourceInput")) {
        handleSourceGossip(check_and_cast<Gossip *>(msg));
    } else {
        error("unhandled message");
    }
}

void GossipHandler::handleExternalGossip(Gossip *gossip) {
    int sender = gossip->getSender();
    int content_id = gossip->getContentId();
    int hops = gossip->getHops();
    simtime_t creation_time = gossip->getCreationTime();
    int bit_length = gossip->getBitLength();

    bool is_new = !cache->contains(content_id);

    if (is_new) {
        EV_DEBUG << "received new gossip with ID " << content_id << " from " << sender << endl;
        emit(new_gossip_received_signal, simTime() - creation_time);
        cache->insert(content_id, creation_time, bit_length);

        // eager multicast new gossip
        Gossip *new_gossip = new Gossip();
        new_gossip->setContentId(content_id);
        new_gossip->setHops(hops + 1);
        new_gossip->setCreationTime(creation_time);
        new_gossip->setBitLength(bit_length);

        for (auto receiver : peer_tracker->eager_peers) {
            if (receiver != sender) {
                Gossip *dup_msg = new_gossip->dup();
                dup_msg->setReceiver(receiver);
                send(dup_msg, "out");
            }
        }
        delete new_gossip;

        // schedule notifications for lazy peers
        for (int peer_id : peer_tracker->lazy_peers) {
            if (peer_id != sender) {
                receivers_to_content_ids[peer_id].insert(content_id);
            }
        }

        // make peer eager
        if (peer_tracker->isPeer(sender)) {
            peer_tracker->last_gossip_time[sender] = simTime();
            peer_tracker->makeEager(sender);
        }
    } else {
        EV_DEBUG << "received known gossip with id " << content_id << " from " << sender << endl;

        if (peer_tracker->isPeer(sender)
            && peer_tracker->last_gossip_time[sender] > simTime() + min_eager_time) {
            peer_tracker->makeLazy(sender);

            Prune2 *prune = new Prune2();
            prune->setReceiver(sender);
            send(prune, "out");
        }
    }

    delete gossip;
}

void GossipHandler::handleSourceGossip(Gossip *gossip) {
    int content_id = gossip->getContentId();
    if (cache->contains(content_id)) {
        error("Source created gossip with used id");
    }

    EV_DEBUG << "emitting new gossip with content id " << content_id << endl;
    emit(new_gossip_received_signal, simTime() - gossip->getCreationTime());

    // add to cache
    cache->insert(content_id, gossip->getCreationTime(), gossip->getBitLength());

    // multicast to eager peers
    for (auto receiver : peer_tracker->eager_peers) {
        Gossip *forwarded_gossip = gossip->dup();
        forwarded_gossip->setReceiver(receiver);
        send(forwarded_gossip, "out");
    }

    // schedule notifications for lazy peers
    for (int peer_id : peer_tracker->lazy_peers) {
        receivers_to_content_ids[peer_id].insert(content_id);
    }

    delete gossip;
}

void GossipHandler::handleScheduler(cMessage *scheduler_msg) {
    for (auto entry : receivers_to_content_ids) {
        int receiver = entry.first;
        std::set<int> content_ids = entry.second;

        if (content_ids.size() > 0) {
            // TODO: should also consider received IHAVEs
            EV_DEBUG << "notifying " << receiver << " about " << content_ids.size()
                << " gossip messages" << endl;

            IHave2 *i_have = new IHave2();
            i_have->setReceiver(receiver);
            i_have->setContentIdsArraySize(content_ids.size());
            int i = 0;
            for (auto content_id : content_ids) {
                i_have->setContentIds(i, content_id);
                i++;
            }
            send(i_have, "out");
        }
    }

    receivers_to_content_ids.clear();
    scheduleAt(simTime() + notification_interval, scheduler_msg);
}
