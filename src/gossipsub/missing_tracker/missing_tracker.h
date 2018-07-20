#ifndef GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_
#define GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_

#include <omnetpp.h>
#include <queue>
#include <map>
#include "../packets_m.h"
#include "../internal_messages_m.h"

using namespace omnetpp;


class MissingTracker : public cSimpleModule {
  private:
    double wait_time;

    std::map<int, std::queue<int>> custodians;
    std::map<int, simtime_t> first_seen_times;

    void handleScheduler(cMessage *msg);
    void handleIHave(IHave *msg);
    void handleCacheQueryResponse(CacheQueryResponse *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_
