#include <omnetpp.h>
#include "../pushpull_packets_m.h"
#include "pull_handler.h"

using namespace omnetpp;


Define_Module(PullHandler);


void PullHandler::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));
}

void PullHandler::handleMessage(cMessage *msg) {
    Pull *pull = check_and_cast<Pull *>(msg);
    int sender = pull->getSender();

    for (int i = 0; i < pull->getContentIdsArraySize(); i++) {
        int content_id = pull->getContentIds(i);
        if (cache->contains(content_id)) {
            EV_DEBUG << "received PULL from " << sender << " for available message " << content_id
                << endl;

            Gossip *gossip = new Gossip();
            gossip->setReceiver(sender);
            gossip->setContentId(content_id);
            gossip->setCreationTime(cache->getCreationTime(content_id));
            send(gossip, "out");
        } else {
            EV_DEBUG << "received PULL from " << sender << " for unavailable message "
                << content_id << endl;
        }
    }

    delete pull;
}
