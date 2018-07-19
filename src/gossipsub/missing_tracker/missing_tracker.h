#ifndef GOSSIPSUB_MISSINGTRACKER_H_
#define GOSSIPSUB_MISSINGTRACKER_H_

#include <queue>
#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"

using namespace omnetpp;


class MissingTracker : public cSimpleModule
{
  private:
      std::map<int, std::queue<int>> content_ids_to_custodians;

      void handleScheduler(cMessage *msg);
      void handleIHave(IHave *msg);
      void handleCacheQueryResponse(CacheQueryResponse *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MISSINGTRACKER_H
