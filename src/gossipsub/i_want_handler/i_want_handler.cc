#include "i_want_handler.h"

using namespace omnetpp;


Define_Module(IWantHandler);


void IWantHandler::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));
}

void IWantHandler::handleMessage(cMessage *msg) {
    IWant *i_want = check_and_cast<IWant *>(msg);
    int sender = i_want->getSender();

    std::vector<int> available_content_ids;

    int num_content_ids = i_want->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = i_want->getContentIds(i);

        if (!cache->contains(content_id)) {
            error("received IWANT for unknown gossip");
        }

        Gossip *gossip = new Gossip();
        gossip->setReceiver(sender);
        gossip->setContentId(content_id);
        gossip->setHops(0);
        gossip->setCreationTime(cache->getCreationTime(content_id));
        gossip->setBitLength(cache->getBitLength(content_id));
        send(gossip, "out");
    }

    delete i_want;
}
