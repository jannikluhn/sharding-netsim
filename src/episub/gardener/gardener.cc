#include <omnetpp.h>
#include "../../utils/cache/cache.h"
#include "../peer_tracker/peer_tracker.h"
#include "gardener.h"

using namespace omnetpp;


Define_Module(Gardener);


void Gardener::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    const char *peer_tracker_path = par("peerTrackerPath").stringValue();
    peer_tracker = check_and_cast<PeerTracker *>(getModuleByPath(peer_tracker_path));

    new_gossip_received_signal = registerSignal("newGossipReceived");
}


void Gardener::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("graftInput")) {
        handleGraft(check_and_cast<Graft2 *>(msg));
    } else if (msg->arrivedOn("pruneInput")) {
        handlePrune(check_and_cast<Prune2 *>(msg));
    } else {
        error("unhandled message");
    }
}

void Gardener::handleGraft(Graft2 *graft) {
    int sender = graft->getSender();
    int num_content_ids = graft->getContentIdsArraySize();
    EV_DEBUG << "received GRAFT from " << sender << " with " << num_content_ids << " content ids"
        << endl;

    for (int i = 0; i < num_content_ids; i++) {
        Gossip *gossip = new Gossip();
        gossip->setHops(0);
        gossip->setReceiver(sender);

        int content_id = graft->getContentIds(i);
        if (!cache->contains(content_id)) {
            error("Received GRAFT for content we do not have");
        }
        gossip->setContentId(content_id);
        gossip->setCreationTime(cache->getCreationTime(content_id));
        gossip->setBitLength(cache->getBitLength(content_id));

        send(gossip, "out");
    }

    peer_tracker->makeEager(sender);
    delete graft;
}

void Gardener::handlePrune(Prune2 *prune) {
    int sender = prune->getSender();
    EV_DEBUG << "received PRUNE from " << sender << endl;
    peer_tracker->makeLazy(sender);
    delete prune;
}
