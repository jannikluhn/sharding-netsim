#ifndef GOSSIPSUB_CACHE_CACHE_H_
#define GOSSIPSUB_CACHE_CACHE_H_

#include <omnetpp.h>
#include <set>

using namespace omnetpp;


class Cache : public cSimpleModule {
  private:
    int node_id;

    std::set<int> content_ids;

    simsignal_t new_gossip_received_signal;

    void handleAddGossip(Gossip *msg);
    void handleQuery(CacheQuery *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_CACHE_CACHE_H_
