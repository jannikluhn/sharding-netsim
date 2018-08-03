#ifndef FLOODSUB_FLOODER_FLOODER_H_
#define FLOODSUB_FLOODER_FLOODER_H_

#include <omnetpp.h>
#include <set>
#include "../../packets_m.h"
#include "../../hyparview/internal_messages_m.h"
#include "../../utils/cache/cache.h"

using namespace omnetpp;


class Flooder : public cSimpleModule {
  private:
    Cache *cache;

    std::set<int> peers;

    simsignal_t new_gossip_received_signal;

    void handleSourceGossip(Gossip *gossip);
    void handleExternalGossip(Gossip *gossip);
    void handleAddedActivePeer(ActiveListChange *active_list_change);
    void handleRemovedActivePeer(ActiveListChange *active_list_change);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // FLOODSUB_FLOODER_FLOODER_H_
