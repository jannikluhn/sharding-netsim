#include "gossiper.h"

using namespace omnetpp;


Define_Module(Gossiper);


void Gossiper::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    const char *overlay_manager_path = par("overlayManagerPath").stringValue();
    overlay_manager = check_and_cast<OverlayManager *>(getModuleByPath(overlay_manager_path));

    heartbeat_interval = par("heartbeatInterval").doubleValue();
    target_mesh_degree = par("targetMeshDegree").intValue();

    new_gossip_received_signal = registerSignal("newGossipReceived");

    cMessage *heartbeat = new cMessage();
    scheduleAt(simTime() + intuniform(0, heartbeat_interval), heartbeat);
}

void Gossiper::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("sourceInput")) {
        handleSourceGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("externalGossipInput")) {
        handleExternalGossip(check_and_cast<Gossip *>(msg));
    } else {
        error("unhandled message");
    }
}

void Gossiper::handleHeartbeat(cMessage *heartbeat) {
    IHave *i_have = new IHave();
    i_have->setContentIdsArraySize(window.size());
    int i = 0;
    for (auto content_id : window) {
        i_have->setContentIds(i, content_id);
        i++;
    }

    if (i > 0) {
        int i_have_receivers = 0;
        for (auto peer : overlay_manager->getNonMeshPeerShuffling()) {
            IHave *i_have_dup = i_have->dup();
            i_have_dup->setReceiver(peer);
            send(i_have_dup, "out");
            EV_DEBUG << "Sent IHAVE about " << window.size() << " content ids to " << peer
                << endl;
            i_have_receivers++;
            if (i_have_receivers >= target_mesh_degree) {
                break;
            }
        }
    }

    delete i_have;
    window.clear();

    scheduleAt(simTime() + heartbeat_interval, heartbeat);
}

void Gossiper::handleSourceGossip(Gossip *gossip) {
    int content_id = gossip->getContentId();
    if (cache->contains(content_id)) {
        error("Source created gossip with used id");
    }

    cache->insert(content_id, gossip->getCreationTime(), gossip->getBitLength());
    window.insert(content_id);
    EV_DEBUG << "emitting new gossip with content id " << content_id << endl;
    emit(new_gossip_received_signal, simTime() - gossip->getCreationTime());

    for (auto peer : overlay_manager->mesh_peers) {
        EV_DEBUG << "sending new gossip with id " << content_id << " to " << peer << endl;
        Gossip *forwarded_gossip = gossip->dup();
        forwarded_gossip->setReceiver(peer);
        send(forwarded_gossip, "out");
    }
    delete gossip;
}

void Gossiper::handleExternalGossip(Gossip *gossip) {
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
        window.insert(content_id);

        Gossip *forwarded_gossip = new Gossip();
        forwarded_gossip->setHops(hops + 1);
        forwarded_gossip->setContentId(content_id);
        forwarded_gossip->setCreationTime(creation_time);
        forwarded_gossip->setBitLength(bit_length);
        for (auto peer : overlay_manager->mesh_peers) {
            if (peer != sender) {
                Gossip *dup_msg = forwarded_gossip->dup();
                dup_msg->setReceiver(peer);
                send(dup_msg, "out");
            }
        }
        delete forwarded_gossip;
    } else {
        EV_DEBUG << "received known gossip with ID " << content_id << " from " << sender << endl;
    }

    delete gossip;
}
