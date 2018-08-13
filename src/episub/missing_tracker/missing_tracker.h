#ifndef EPISUB_MISSING_TRACKER_MISSING_TRACKER_H_
#define EPISUB_MISSING_TRACKER_MISSING_TRACKER_H_

#include <omnetpp.h>
#include <queue>
#include <map>
#include "../../utils/cache/cache.h"
#include "../../packets_m.h"

using namespace omnetpp;


class MissingTracker : public cSimpleModule {
  private:
    Cache *cache;

    double wait_time;

    simsignal_t missing_signal;

    std::set<int> they_have_content_ids;
    std::map<int, std::queue<int>> custodians;
    std::map<int, simtime_t> first_seen_timestamps;

    void handleScheduler(cMessage *msg);
    void handleIHave(IHave *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // EPISUB_MISSING_TRACKER_MISSING_TRACKER_H_
