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
    } else if (msg->arrivedOn("peerListChangeInput")) {
        handlePeerListChange(check_and_cast<PeerListChange *>(msg));        
    } else {
        error("Unhandled message");
    }
}

void Flooder::handleSourceGossip(Gossip *gossip) {
    int content_id = gossip->getContentId();
    if (cache->contains(content_id)) {
        error("Source created gossip with used id");
    }

    EV_DEBUG << "emitting new gossip with content id " << content_id << endl;
    emit(new_gossip_received_signal, simTime() - gossip->getCreationTime());
    cache->insert(content_id, gossip->getCreationTime());

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
    simtime_t creation_time = gossip->getCreationTime();

    int content_id = gossip->getContentId();
    if (!cache->contains(content_id)) {
        EV_DEBUG << "received new gossip with id " << content_id << " from " << sender << endl;
        emit(new_gossip_received_signal, simTime() - gossip->getCreationTime());
        cache->insert(content_id, gossip->getCreationTime());

        Gossip *new_gossip = new Gossip();
        new_gossip->setHops(hops + 1);
        new_gossip->setContentId(content_id);
        new_gossip->setCreationTime(creation_time);
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

void Flooder::handlePeerListChange(PeerListChange *peer_list_change) {
    for (int i = 0; i < peer_list_change->getAddedPeersArraySize(); i++) {
        peers.insert(peer_list_change->getAddedPeers(i));
    }
    for (int i = 0; i < peer_list_change->getRemovedPeersArraySize(); i++) {
        peers.insert(peer_list_change->getRemovedPeers(i));
    }
    delete peer_list_change;
}
