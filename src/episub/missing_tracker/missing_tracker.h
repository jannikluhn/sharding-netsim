#ifndef EPISUB_MISSING_TRACKER_MISSING_TRACKER_H_
#define EPISUB_MISSING_TRACKER_MISSING_TRACKER_H_

#include <omnetpp.h>
#include <queue>
#include <map>
#include "../../utils/cache/cache.h"
#include "../episub_packets_m.h"

using namespace omnetpp;


class MissingTracker : public cSimpleModule {
  private:
    Cache *cache;

    double request_wait_time;
    double request_round_trip_bound;
    double heartbeat_interval;

    std::set<int> they_have_content_ids;
    std::map<int, std::queue<int>> custodians;
    std::map<int, simtime_t> next_request_timestamps;

    void handleHeartbeat(cMessage *heartbeat);
    void handleIHave(IHave2 *i_have);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // EPISUB_MISSING_TRACKER_MISSING_TRACKER_H_
