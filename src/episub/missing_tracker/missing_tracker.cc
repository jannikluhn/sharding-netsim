#include <omnetpp.h>
#include "missing_tracker.h"

using namespace omnetpp;


Define_Module(MissingTracker);


void MissingTracker::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    request_wait_time = par("requestWaitTime").doubleValue();
    request_round_trip_bound = par("requestRoundTripBound").doubleValue();
    heartbeat_interval = request_round_trip_bound / 5;

    cMessage *heartbeat = new cMessage();
    scheduleAt(simTime() + uniform(0, heartbeat_interval), heartbeat);
}

void MissingTracker::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("iHaveInput")) {
        handleIHave(check_and_cast<IHave2 *>(msg));
        delete msg;
    } else {
        error("unhandled message");
    }
}

void MissingTracker::handleHeartbeat(cMessage *heartbeat) {
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
        next_request_timestamps.erase(content_id);
    }

    // select content ids to attempt to download
    std::set<int> content_to_retrieve;
    for (auto content_id : they_have_content_ids) {
        if (next_request_timestamps[content_id] < simTime()) {
            if (custodians[content_id].empty()) {
                EV_WARN << "probably failed to retrieve content with id " << content_id << endl;

                they_have_content_ids.erase(content_id);
                custodians.erase(content_id);
                next_request_timestamps.erase(content_id);
            } else {
                content_to_retrieve.insert(content_id);
            }
        }
    }

    // find custodian for each content id
    std::map<int, std::vector<int>> content_ids_by_custodian;
    for (auto content_id : content_to_retrieve) {
        if (custodians[content_id].empty()) {
            break;
        }
        int next_custodian = custodians[content_id].front();
        custodians[content_id].pop();

        content_ids_by_custodian[next_custodian].push_back(content_id);
    }

    // send GRAFT to custodians
    for (auto entry : content_ids_by_custodian) {
        int custodian = entry.first;
        std::vector<int> content_ids = entry.second;

        EV_DEBUG << "requesting " << content_ids.size() << " missed gossip messages from "
            << custodian << endl;

        Graft2 *graft_msg = new Graft2();
        graft_msg->setReceiver(custodian);
        graft_msg->setContentIdsArraySize(content_ids.size());
        for (int i = 0; i < content_ids.size(); i++) {
            graft_msg->setContentIds(i, content_ids[i]);
            next_request_timestamps[content_ids[i]] = simTime() + (i + 1) * request_round_trip_bound;
        }

        send(graft_msg, "out");
    }

    scheduleAt(simTime() + heartbeat_interval, heartbeat);
}

void MissingTracker::handleIHave(IHave2 *i_have) {
    int num_content_ids = i_have->getContentIdsArraySize();
    for (int i = 0; i < num_content_ids; i++) {
        int content_id = i_have->getContentIds(i);
        EV_DEBUG << "received IHAVE with gossip id " << content_id << " from "
            << i_have->getSender() << endl;

        if (they_have_content_ids.count(content_id) == 0) {
            they_have_content_ids.insert(content_id);
            next_request_timestamps[content_id] = simTime() + request_wait_time;
        }
        custodians[content_id].push(i_have->getSender());
    }
}
