#ifndef GOSSIPSUB_SOURCE_SOURCE_H_
#define GOSSIPSUB_SOURCE_SOURCE_H_

#include <omnetpp.h>
#include "../cache/cache.h"

using namespace omnetpp;


class Source : public cSimpleModule {
  private:
    Cache *cache;

    int node_id;
    bool active;
    double start_time;
    double stop_time;
    double rate;
    simsignal_t new_gossip_emitted_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_SOURCE_SOURCE_H_
