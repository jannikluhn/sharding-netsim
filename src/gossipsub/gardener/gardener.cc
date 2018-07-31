#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "gardener.h"

using namespace omnetpp;


Define_Module(Gardener);


void Gardener::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("graftInput")) {
        handleGraft(check_and_cast<Graft *>(msg));
    } else if (msg->arrivedOn("pruneInput")) {
        handlePrune(check_and_cast<Prune *>(msg));
    } else if (msg->arrivedOn("controlInput")) {
        handleControl(check_and_cast<GardenerControl *>(msg));
    } else if (msg->arrivedOn("eagerMulticastInputs")) {
        handleEagerMulticast(check_and_cast<AddressedPacket *>(msg));
    } else if (msg->arrivedOn("lazyMulticastInputs")) {
        handleLazyMulticast(check_and_cast<AddressedPacket *>(msg));
    } else if (msg->arrivedOn("addedActivePeerInput")) {
        handleAddedActivePeer(check_and_cast<ActiveListChange *>(msg));
    } else if (msg->arrivedOn("removedActivePeerInput")) {
        handleRemovedActivePeer(check_and_cast<ActiveListChange *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }
    delete msg;
}

void Gardener::handleGraft(Graft *msg) {
    // assume for the simulation that we know the message (otherwise an honest peer wouldn't have
    // sent GRAFT)
    Gossip *gossip_msg = new Gossip();
    gossip_msg->setReceiver(msg->getSender());

    int num_content_ids = msg->getContentIdsArraySize();
    gossip_msg->setContentIdsArraySize(num_content_ids);
    for (int i = 0; i < num_content_ids; i++) {
        gossip_msg->setContentIds(i, msg->getContentIds(i));
    }
    gossip_msg->setHops(0);
    send(gossip_msg, "out");

    int sender_id = msg->getSender();
    eager_receivers.insert(sender_id);
    lazy_receivers.erase(sender_id);
}

void Gardener::handlePrune(Prune *msg) {
    int sender_id = msg->getSender();
    lazy_receivers.insert(sender_id);
    eager_receivers.erase(sender_id);
}

void Gardener::handleControl(GardenerControl *msg) {
    for (int i = 0; i < msg->getGraftReceiversArraySize(); i++) {
        int receiver_id = msg->getGraftReceivers(i);
        eager_receivers.insert(receiver_id);
        lazy_receivers.erase(receiver_id);
    }
    for (int i = 0; i < msg->getPruneReceiversArraySize(); i++) {
        int receiver_id = msg->getPruneReceivers(i);
        if (eager_receivers.find(receiver_id) != eager_receivers.end()) {
            eager_receivers.erase(receiver_id);
            lazy_receivers.erase(receiver_id);
            Prune *prune_msg = new Prune();
            prune_msg->setReceiver(receiver_id);
            send(prune_msg, "out");
        }
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
