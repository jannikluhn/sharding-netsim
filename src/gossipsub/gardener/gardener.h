#ifndef GOSSIPSUB_GARDENER_GARDENER_H_
#define GOSSIPSUB_GARDENER_GARDENER_H_

#include <omnetpp.h>
#include <set>
#include "../cache/cache.h"

using namespace omnetpp;


class Gardener : public cSimpleModule {
  private:
    Cache *cache;

    std::set<int> lazy_receivers;
    std::set<int> eager_receivers;

    simsignal_t new_gossip_received_signal;

    void prune(int receiver_id);
    void graft(int receiver_id);

    void handleGraft(Graft *msg);
    void handlePrune(Prune *msg);
    void handleGossip(Gossip *msg);
    void handleEagerMulticast(AddressedPacket *msg);
    void handleLazyMulticast(AddressedPacket *msg);
    void handleAddedActivePeer(ActiveListChange *msg);
    void handleRemovedActivePeer(ActiveListChange *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_GARDENER_GARDENER_H_
