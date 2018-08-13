#ifndef EPISUB_GARDENER_GARDENER_H_
#define EPISUB_GARDENER_GARDENER_H_

#include <omnetpp.h>
#include <set>
#include "../../packets_m.h"
#include "../peer_tracker/peer_tracker.h"
#include "../../utils/cache/cache.h"

using namespace omnetpp;


class Gardener : public cSimpleModule {
  private:
    Cache *cache;
    PeerTracker *peer_tracker;

    simsignal_t new_gossip_received_signal;

    void handleGraft(Graft *msg);
    void handlePrune(Prune *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // EPISUB_GARDENER_GARDENER_H_
