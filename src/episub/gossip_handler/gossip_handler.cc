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
    std::set<int> new_content_ids;

    // insert content ids into cache and note which ones are new
    int n = gossip->getContentIdsArraySize();
    for (int i = 0; i < n; i++) {
        int content_id = gossip->getContentIds(i);
        if (!cache->contains(content_id)) {
            cache->insert(content_id);
            new_content_ids.insert(content_id);
            emit(new_gossip_received_signal, gossip->getHops());
        }
    }

    if (!new_content_ids.empty()) {
        EV_DEBUG << "received new gossip from " << sender << endl;

        // eager multicast new gossip
        Gossip *new_gossip = new Gossip();

        new_gossip->setHops(gossip->getHops() + 1);

        new_gossip->setContentIdsArraySize(new_content_ids.size());
        int i = 0;
        for (auto content_id : new_content_ids) {
            new_gossip->setContentIds(i, content_id);
            i++;
        }

        for (auto receiver : peer_tracker->eager_peers) {
            if (receiver != sender) {
                Gossip *dup_msg = new_gossip->dup();
                dup_msg->setReceiver(receiver);
                send(dup_msg, "out");
            }
        }
        delete new_gossip;

        // schedule notifications for lazy peers
        for (auto content_id : new_content_ids) {
            for (int peer_id : peer_tracker->lazy_peers) {
                if (peer_id != sender) {
                    receivers_to_content_ids[peer_id].insert(content_id);
                }
            }
        }

        if (peer_tracker->isPeer(sender)) {
            peer_tracker->makeEager(sender);
        }
    } else {
        EV_DEBUG << "received known gossip from " << sender << endl;

        if (peer_tracker->isPeer(sender)) {
            peer_tracker->makeLazy(sender);
        }

        Prune2 *prune = new Prune2();
        prune->setReceiver(sender);
        send(prune, "out");
    }

    delete gossip;
}

void GossipHandler::handleSourceGossip(Gossip *gossip) {
    // add to cache
    for (int i = 0; i < gossip->getContentIdsArraySize(); i++) {
        int content_id = gossip->getContentIds(i);
        cache->insert(content_id);
    }

    // multicast to eager peers
    for (auto receiver : peer_tracker->eager_peers) {
        Gossip *forwarded_gossip = gossip->dup();
        forwarded_gossip->setReceiver(receiver);
        send(forwarded_gossip, "out");
    }

    // schedule notifications for lazy peers
    for (int i = 0; i < gossip->getContentIdsArraySize(); i++) {
        int content_id = gossip->getContentIds(i);
        for (int peer_id : peer_tracker->lazy_peers) {
            receivers_to_content_ids[peer_id].insert(content_id);
        }
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
