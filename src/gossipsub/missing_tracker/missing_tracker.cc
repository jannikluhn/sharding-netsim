#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "missing_tracker.h"

using namespace omnetpp;


Define_Module(MissingTracker);


void MissingTracker::initialize()
{
    cMessage *schedulerMsg = new cMessage();
    scheduleAt(simTime() + 2.5, schedulerMsg);
}

void MissingTracker::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleScheduler(msg);
    } else if (msg->arrivedOn("iHaveInput")) {
        handleIHave(check_and_cast<IHave *>(msg));
        delete msg;
    } else if (msg->arrivedOn("cacheQueryPort")) {
        handleCacheQueryResponse(check_and_cast<CacheQueryResponse *>(msg));
        delete msg;
    }
}

void MissingTracker::handleScheduler(cMessage *msg)
{
    for (auto entry : content_ids_to_custodians) {
        int content_id = entry.first;
        CacheQuery *cache_query = new CacheQuery();
        cache_query->setContentId(content_id);
        send(cache_query, "cacheQueryPort$o");
    }

    scheduleAt(simTime() + 2.5, msg);
}

void MissingTracker::handleIHave(IHave *msg)
{
    int num_content_ids = msg->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int contentId = msg->getContentIds(i);

        if (content_ids_to_custodians.count(contentId) == 0) {
            content_ids_to_custodians[contentId] = std::queue<int>();
        }
        content_ids_to_custodians[contentId].push(msg->getSender());
    }
}

void MissingTracker::handleCacheQueryResponse(CacheQueryResponse *msg)
{
    int found = msg->getFound();
    int content_id = msg->getContentId();

    if (found) {
        content_ids_to_custodians.erase(content_id);
    } else {
        int next_custodian = content_ids_to_custodians[content_id].front();
        content_ids_to_custodians[content_id].pop();

        Graft *graft_msg = new Graft();
        graft_msg->setContentIdsArraySize(1);
        graft_msg->setContentIds(0, content_id);
        graft_msg->setReceiver(next_custodian);
        send(graft_msg, "out");
    }
}
