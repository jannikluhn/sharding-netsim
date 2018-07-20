#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "missing_tracker.h"

using namespace omnetpp;


Define_Module(MissingTracker);


void MissingTracker::initialize() {
    wait_time = par("waitTime").doubleValue();
    cMessage *scheduler_msg = new cMessage();
    scheduleAt(simTime() + uniform(0, wait_time / 2), scheduler_msg);
}

void MissingTracker::handleMessage(cMessage *msg) {
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

void MissingTracker::handleScheduler(cMessage *msg) {
    for (auto entry : custodians) {
        int content_id = entry.first;

        if (first_seen_times[content_id] < simTime() - wait_time) {
            CacheQuery *cache_query = new CacheQuery();
            cache_query->setContentId(content_id);
            send(cache_query, "cacheQueryPort$o");
        }
    }

    scheduleAt(simTime() + wait_time / 2, msg);
}

void MissingTracker::handleIHave(IHave *msg) {
    int num_content_ids = msg->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = msg->getContentIds(i);

        if (custodians.count(content_id) == 0) {
            custodians[content_id] = std::queue<int>();
            first_seen_times[content_id] = simTime();
        }
        custodians[content_id].push(msg->getSender());
    }
}

void MissingTracker::handleCacheQueryResponse(CacheQueryResponse *msg) {
    int found = msg->getFound();
    int content_id = msg->getContentId();

    if (found) {
        custodians.erase(content_id);
        first_seen_times.erase(content_id);
    } else {
        int next_custodian = custodians[content_id].front();
        custodians[content_id].pop();
        if (custodians[content_id].empty()) {
            first_seen_times.erase(content_id);
        }

        Graft *graft_msg = new Graft();
        graft_msg->setContentIdsArraySize(1);
        graft_msg->setContentIds(0, content_id);
        graft_msg->setReceiver(next_custodian);
        send(graft_msg, "out");
    }
}
