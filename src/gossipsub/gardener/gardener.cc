#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "gardener.h"

using namespace omnetpp;


Define_Module(Gardener);


void Gardener::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    new_gossip_received_signal = registerSignal("newGossipReceived");
}


void Gardener::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("graftInput")) {
        handleGraft(check_and_cast<Graft *>(msg));
    } else if (msg->arrivedOn("pruneInput")) {
        handlePrune(check_and_cast<Prune *>(msg));
    } else if (msg->arrivedOn("gossipInput")) {
        handleGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("eagerMulticastInputs")) {
        handleEagerMulticast(check_and_cast<AddressedPacket *>(msg));
    } else if (msg->arrivedOn("lazyMulticastInputs")) {
        handleLazyMulticast(check_and_cast<AddressedPacket *>(msg));
    } else if (msg->arrivedOn("addedActivePeerInput")) {
        handleAddedActivePeer(check_and_cast<ActiveListChange *>(msg));
    } else if (msg->arrivedOn("removedActivePeerInput")) {
        handleRemovedActivePeer(check_and_cast<ActiveListChange *>(msg));
    } else {
        error("unhandled message");
    }
    delete msg;
}

void Gardener::handleGraft(Graft *msg) {
    EV_DEBUG << "received GRAFT from " << msg->getSender() << endl;

    Gossip *gossip_msg = new Gossip();
    gossip_msg->setReceiver(msg->getSender());

    int num_content_ids = msg->getContentIdsArraySize();
    gossip_msg->setContentIdsArraySize(num_content_ids);
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = msg->getContentIds(i);
        if (!cache->contains(content_id)) {
            EV_ERROR << content_id << endl;
            error("Received GRAFT for content we do not have");
        }
        gossip_msg->setContentIds(i, content_id);
    }
    gossip_msg->setHops(0);
    send(gossip_msg, "out");

    int sender_id = msg->getSender();
    eager_receivers.insert(sender_id);
    lazy_receivers.erase(sender_id);
}

void Gardener::handlePrune(Prune *msg) {
    int sender_id = msg->getSender();
    EV_DEBUG << "received PRUNE from " << sender_id << endl;
    lazy_receivers.insert(sender_id);
    eager_receivers.erase(sender_id);
}

void Gardener::handleGossip(Gossip *msg) {
    int sender = msg->getSender();
    std::set<int> new_content_ids;

    // insert content ids into cache and note which ones are new
    int n = msg->getContentIdsArraySize();
    for (int i = 0; i < n; i++) {
        int content_id = msg->getContentIds(i);
        if (!cache->contains(content_id)) {
            new_content_ids.insert(content_id);
            emit(new_gossip_received_signal, msg->getHops());
        }
        cache->insert(content_id);
    }

    if (!new_content_ids.empty()) {
        EV_DEBUG << "received new gossip from " << sender << endl;

        // eager multicast new gossip
        Gossip *new_gossip_msg = new Gossip();

        new_gossip_msg->setHops(msg->getHops() + 1);

        new_gossip_msg->setContentIdsArraySize(new_content_ids.size());
        int i = 0;
        for (auto content_id : new_content_ids) {
            new_gossip_msg->setContentIds(i, content_id);
            i++;
        }

        for (auto receiver_id : eager_receivers) {
            if (receiver_id != sender) {
                Gossip *dup_msg = new_gossip_msg->dup();
                dup_msg->setReceiver(receiver_id);
                send(dup_msg, "out");
            }
        }
        delete new_gossip_msg;

        // make sender eager
        eager_receivers.insert(sender);
        lazy_receivers.erase(sender);
    } else {
        EV_DEBUG << "received known gossip from " << sender << endl;

        // make sender lazy
        lazy_receivers.insert(sender);
        eager_receivers.erase(sender);

        Prune *prune = new Prune();
        prune->setReceiver(sender);
        send(prune, "out");
    }
}

void Gardener::handleEagerMulticast(AddressedPacket *msg) {
    int relayer_id = msg->getSender();
    for (auto receiver_id : eager_receivers) {
        if (receiver_id != relayer_id) {
            AddressedPacket *dup_msg = msg->dup();
            dup_msg->setReceiver(receiver_id);
            send(dup_msg, "out");
        }
    }
}

void Gardener::handleLazyMulticast(AddressedPacket *msg) {
    for (auto receiver_id : lazy_receivers) {
        AddressedPacket *dup_msg = msg->dup();
        dup_msg->setReceiver(receiver_id);
        send(dup_msg, "out");
    }
}

void Gardener::handleAddedActivePeer(ActiveListChange *msg) {
    eager_receivers.insert(msg->getAdded());
}

void Gardener::handleRemovedActivePeer(ActiveListChange *msg) {
    int peer = msg->getRemoved();
    eager_receivers.erase(peer);
    lazy_receivers.erase(peer);
}
