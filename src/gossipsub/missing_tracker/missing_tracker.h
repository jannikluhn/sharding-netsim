#ifndef GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_
#define GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_

#include <omnetpp.h>
#include <queue>
#include <map>
#include "../cache/cache.h"
#include "../../packets_m.h"

using namespace omnetpp;


class MissingTracker : public cSimpleModule {
  private:
    Cache *cache;

    double wait_time;

    std::set<int> they_have_content_ids;
    std::map<int, std::queue<int>> custodians;
    std::map<int, simtime_t> first_seen_timestamps;

    void handleScheduler(cMessage *msg);
    void handleIHave(IHave *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_
