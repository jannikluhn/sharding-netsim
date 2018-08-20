#ifndef UTILS_SOURCE_SOURCE_H_
#define UTILS_SOURCE_SOURCE_H_

#include <omnetpp.h>

using namespace omnetpp;


class Source : public cSimpleModule {
  private:
    bool active;
    simtime_t start_time;
    simtime_t stop_time;
    double gossip_rate;
    simsignal_t new_gossip_emitted_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_SOURCE_SOURCE_H_
