#ifndef GOSSIPSUB_GARDENER_GARDENER_H_
#define GOSSIPSUB_GARDENER_GARDENER_H_

#include <omnetpp.h>
#include <set>

using namespace omnetpp;


class Gardener : public cSimpleModule {
  private:
    std::set<int> lazy_receivers;
    std::set<int> eager_receivers;

    void prune(int receiver_id);
    void graft(int receiver_id);

    void handleGraft(Graft *msg);
    void handlePrune(Prune *msg);
    void handleControl(GardenerControl *msg);
    void handleEagerMulticast(AddressedPacket *msg);
    void handleLazyMulticast(AddressedPacket *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_GARDENER_GARDENER_H_
