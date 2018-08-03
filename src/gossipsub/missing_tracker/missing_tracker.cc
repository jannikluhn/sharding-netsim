#include <omnetpp.h>
#include "../../packets_m.h"
#include "missing_tracker.h"

using namespace omnetpp;


Define_Module(MissingTracker);


void MissingTracker::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

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
    } else {
        error("unhandled message");
    }
}

void MissingTracker::handleScheduler(cMessage *msg) {
    // remove available content from list
    std::set<int> available_content_ids;
    for (auto content_id : they_have_content_ids) {
        if (cache->contains(content_id)) {
            available_content_ids.insert(content_id);
        }
    }
    for (auto content_id : available_content_ids) {
        they_have_content_ids.erase(content_id);
        custodians.erase(content_id);
        first_seen_timestamps.erase(content_id);
    }

    // select content ids to attempt to download
    std::set<int> content_to_retrieve;
    for (auto content_id : they_have_content_ids) {
        if (first_seen_timestamps[content_id] < simTime() - wait_time) {
            content_to_retrieve.insert(content_id);
        }
    }


    // find custodian for each content id
    std::map<int, std::vector<int>> content_ids_by_custodian;
    for (auto content_id : content_to_retrieve) {
        if (custodians[content_id].empty()) {
            error("failed to retrieve content");
            // TODO: could also be that request is still pending (doesn't matter if
            // wait_time >> latency)
        }
        int next_custodian = custodians[content_id].front();
        custodians[content_id].pop();

        content_ids_by_custodian[next_custodian].push_back(content_id);
    }

    // send GRAFT to custodians
    for (auto entry : content_ids_by_custodian) {
        int custodian = entry.first;
        std::vector<int> content_ids = entry.second;

        EV_DEBUG << "requesting missed gossip messages from " << custodian << endl;

        Graft *graft_msg = new Graft();
        graft_msg->setReceiver(custodian);
        graft_msg->setContentIdsArraySize(content_ids.size());
        for (int i = 0; i < content_ids.size(); i++) {
            graft_msg->setContentIds(i, content_ids[i]);
        }

        send(graft_msg, "out");
    }

    scheduleAt(simTime() + wait_time / 2, msg);
}

void MissingTracker::handleIHave(IHave *msg) {
    int num_content_ids = msg->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = msg->getContentIds(i);

        if (they_have_content_ids.count(content_id) == 0) {
            they_have_content_ids.insert(content_id);
            first_seen_timestamps[content_id] = simTime();
        }
        custodians[content_id].push(msg->getSender());
    }
}
