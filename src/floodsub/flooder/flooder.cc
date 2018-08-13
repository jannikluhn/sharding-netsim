#include <omnetpp.h>
#include "../../packets_m.h"
#include "../../hyparview/internal_messages_m.h"
#include "flooder.h"

using namespace omnetpp;


Define_Module(Flooder);


void Flooder::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    new_gossip_received_signal = registerSignal("newGossipReceived");
}

void Flooder::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("sourceInput")) {
        handleSourceGossip(check_and_cast<Gossip *>(msg));        
    } else if (msg->arrivedOn("gossipInput")) {
        handleExternalGossip(check_and_cast<Gossip *>(msg));        
    } else if (msg->arrivedOn("addedActivePeerInput")) {
        handleAddedActivePeer(check_and_cast<ActiveListChange *>(msg));        
    } else if (msg->arrivedOn("removedActivePeerInput")) {
        handleRemovedActivePeer(check_and_cast<ActiveListChange *>(msg));        
    } else {
        error("Unhandled message");
    }
}

void Flooder::handleSourceGossip(Gossip *gossip) {
    for (int i = 0; i < gossip->getContentIdsArraySize(); i++) {
        int content_id = gossip->getContentIds(i);
        cache->insert(content_id);
    }

    for (auto peer : peers) {
        Gossip *dup = gossip->dup();
        dup->setReceiver(peer);
        send(dup, "out");
    }

    delete gossip;
}

void Flooder::handleExternalGossip(Gossip *gossip) {
    int sender = gossip->getSender();
    int hops = gossip->getHops();

    std::set<int> new_content_ids;
    for (int i = 0; i < gossip->getContentIdsArraySize(); i++) {
        int content_id = gossip->getContentIds(i);
        if (!cache->contains(content_id)) {
            cache->insert(content_id);
            new_content_ids.insert(content_id);
            emit(new_gossip_received_signal, hops);
        }
    }

    if (!new_content_ids.empty()) {
        EV_DEBUG << "received new gossip from " << sender << endl;

        Gossip *new_gossip = new Gossip();
        new_gossip->setContentIdsArraySize(new_content_ids.size());
        int i = 0;
        for (auto content_id : new_content_ids) {
            new_gossip->setContentIds(i, content_id);
            i++;
        }
        new_gossip->setHops(hops + 1);

        for (auto peer : peers) {
            if (peer != sender) {
                Gossip *dup = new_gossip->dup();
                dup->setReceiver(peer);
                send(dup, "out");
            }
        }

        delete new_gossip;
    }

    delete gossip;
}

void Flooder::handleAddedActivePeer(ActiveListChange *active_list_change) {
    peers.insert(active_list_change->getAdded());
    delete active_list_change;
}

void Flooder::handleRemovedActivePeer(ActiveListChange *active_list_change) {
    peers.erase(active_list_change->getRemoved());
    delete active_list_change;
}
