#include "i_have_handler.h"

using namespace omnetpp;


Define_Module(IHaveHandler);


void IHaveHandler::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));
}

void IHaveHandler::handleMessage(cMessage *msg) {
    IHave *i_have = check_and_cast<IHave *>(msg);

    std::vector<int> missing_content_ids;

    int num_content_ids = i_have->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = i_have->getContentIds(i);
        if (!cache->contains(content_id)) {
            missing_content_ids.push_back(content_id);
            EV_DEBUG << "received IHAVE from " << i_have->getSender()
                << " about missing content with id " << content_id << endl;
        }
    }

    if (!missing_content_ids.empty()) {
        EV_DEBUG << "requesting " << missing_content_ids.size()
            << " missing messages with IWANT" << endl;
        IWant *i_want = new IWant();
        i_want->setReceiver(i_have->getSender());
        i_want->setContentIdsArraySize(missing_content_ids.size());
        for (int i = 0; i < missing_content_ids.size(); i++) {
            i_want->setContentIds(i, missing_content_ids[i]);
        }
        send(i_want, "out");
    } else {
        EV_DEBUG << "received IHAVE from " << i_have->getSender() << " with no news" << endl;
    }

    delete i_have;
}
