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

    cMessage *heartbeat = new cMessage();
    scheduleAt(simTime() + heartbeat_interval, heartbeat);
}

void Gossiper::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("internalGossipInput")) {
        handleInternalGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("externalGossipInput")) {
        handleExternalGossip(check_and_cast<Gossip *>(msg));
    } else {
        error("unhandled message");
    }
}

void Gossiper::handleHeartbeat(cMessage *msg) {
    IHave *i_have = new IHave();
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
            i_have_receivers++;
            if (i_have_receivers >= target_mesh_degree) {
                break;
            }
        }
        EV_DEBUG << "Sent IHAVE about " << window.size() << "content ids to "
            << i_have_receivers << " non-mesh peers" << endl;
    } else {
        EV_DEBUG << "Omit IHAve due to lack of new messages" << endl;
    }

    delete i_have;
    window.clear();
}

void Gossiper::handleInternalGossip(Gossip *gossip) {
    for (int i = 0; i < gossip->getContentIdsArraySize(); i++) {
        int content_id = gossip->getContentIds(i);
        cache->insert(content_id);
        window.insert(content_id);
    }

    for (auto peer : overlay_manager->mesh_peers) {
        Gossip *forwarded_gossip = gossip->dup();
        forwarded_gossip->setReceiver(peer);
        send(forwarded_gossip, "out");
    }
    delete gossip;
}

void Gossiper::handleExternalGossip(Gossip *gossip) {
    int sender = gossip->getSender();

    std::set<int> new_gossip;
    for (int i = 0; i < gossip->getContentIdsArraySize(); i++) {
        int content_id = gossip->getContentIds(i);
        if (!cache->contains(content_id)) {
            cache->insert(content_id);
            new_gossip.insert(content_id);
            window.insert(content_id);
        }
    }

    if (!new_gossip.empty()) {
        Gossip *forwarded_gossip = new Gossip();
        int i = 0;
        for (auto content_id : new_gossip) {
            forwarded_gossip->setContentIds(i, content_id);
            i++;
        }
        for (auto peer : overlay_manager->mesh_peers) {
            if (peer != sender) {
                Gossip *gossip_dup = forwarded_gossip->dup();
                gossip_dup->setReceiver(peer);
                send(gossip_dup, "out");
            }
        }
        delete forwarded_gossip;
    }

    delete gossip;
}
