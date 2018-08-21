#include "i_want_handler.h"

using namespace omnetpp;


Define_Module(IWantHandler);


void IWantHandler::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));
}

void IWantHandler::handleMessage(cMessage *msg) {
    IWant *i_want = check_and_cast<IWant *>(msg);

    std::vector<int> available_content_ids;

    int num_content_ids = i_want->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = i_want->getContentIds(i);
        if (cache->contains(content_id)) {
            available_content_ids.push_back(content_id);
        }
    }

    if (!available_content_ids.empty()) {
        Gossip *gossip = new Gossip();
        gossip->setReceiver(i_want->getSender());
        gossip->setContentIdsArraySize(available_content_ids.size());
        for (int i = 0; i < available_content_ids.size(); i++) {
            gossip->setContentIds(i, available_content_ids[i]);
        }
        send(gossip, "out");
    }

    delete i_want;
}
