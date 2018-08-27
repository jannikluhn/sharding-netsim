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

    std::vector<int> available_content_ids;
    for (int i = 0; i < pull->getContentIdsArraySize(); i++) {
        int content_id = pull->getContentIds(i);
        if (cache->contains(content_id)) {
            available_content_ids.push_back(content_id);
            EV_DEBUG << "received PULL from " << sender << " for available message " << content_id
                << endl;
        } else {
            EV_DEBUG << "received PULL from " << sender << " for unavailable message "
                << content_id << endl;
        }
    }

    if (available_content_ids.size() > 0) {
        Gossip *gossip = new Gossip();
        gossip->setReceiver(sender);
        gossip->setContentIdsArraySize(available_content_ids.size());
        for (int i = 0; i < available_content_ids.size(); i++) {
            gossip->setContentIds(i, available_content_ids[i]);
        }
        send(gossip, "out");
    }

    delete msg;
}
