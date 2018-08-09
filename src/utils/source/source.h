#ifndef UTILS_SOURCE_SOURCE_H_
#define UTILS_SOURCE_SOURCE_H_

#include <omnetpp.h>
#include "../cache/cache.h"

using namespace omnetpp;


class Source : public cSimpleModule {
  private:
    Cache *cache;

    int node_id;
    bool active;
    double warmup_time;
    double life_time;
    simtime_t start_time;
    simtime_t stop_time;
    double rate;
    simsignal_t new_gossip_emitted_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_SOURCE_SOURCE_H_
