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
    std::map<int, std::queue<int>> content_ids_to_custodians;

    void handleScheduler(cMessage *msg);
    void handleIHave(IHave *msg);
    void handleCacheQueryResponse(CacheQueryResponse *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MISSING_TRACKER_MISSING_TRACKER_H_
