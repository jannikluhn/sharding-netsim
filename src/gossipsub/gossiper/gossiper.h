#ifndef GOSSIPSUB_GOSSIPER_GOSSIPER_H_
#define GOSSIPSUB_GOSSIPER_GOSSIPER_H_

#include <omnetpp.h>
#include "../overlay_manager/overlay_manager.h"
#include "../../utils/cache/cache.h"
#include "../gossipsub_packets_m.h"

using namespace omnetpp;


class Gossiper : public cSimpleModule {
  private:
    Cache *cache;
    OverlayManager *overlay_manager;

    double heartbeat_interval;
    int target_mesh_degree;

    std::set<int> window;

    simsignal_t new_gossip_received_signal;

    void handleHeartbeat(cMessage *heartbeat);
    void handleSourceGossip(Gossip *gossip);
    void handleExternalGossip(Gossip *gossip);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_GOSSIPER_GOSSIPER_H_
