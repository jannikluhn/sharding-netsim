#ifndef EPISUB_GOSSIP_HANDLER_GOSSIP_HANDLER_H_
#define EPISUB_GOSSIP_HANDLER_GOSSIP_HANDLER_H_

#include <omnetpp.h>
#include <set>
#include "../../packets_m.h"
#include "../peer_tracker/peer_tracker.h"
#include "../../utils/cache/cache.h"

using namespace omnetpp;


class GossipHandler : public cSimpleModule {
  private:
    Cache *cache;
    PeerTracker *peer_tracker;

    double notification_interval;

    simsignal_t new_gossip_received_signal;

    std::map<int, std::set<int>> receivers_to_content_ids;

    void handleScheduler(cMessage *msg);
    void handleExternalGossip(Gossip *gossip);
    void handleInternalGossip(Gossip *gossip);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // EPISUB_GOSSIP_HANDLER_GOSSIP_HANDLER_H_
