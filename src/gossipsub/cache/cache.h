#ifndef GOSSIPSUB_CACHE_CACHE_H_
#define GOSSIPSUB_CACHE_CACHE_H_

#include <omnetpp.h>
#include <set>

using namespace omnetpp;


class Cache : public cSimpleModule {
  private:
    std::set<int> content_ids;
    void handleAddGossip(Gossip *msg);
    void handleQuery(CacheQuery *msg);

  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_CACHE_CACHE_H_
