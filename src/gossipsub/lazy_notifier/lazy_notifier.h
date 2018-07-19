#ifndef GOSSIPSUB_LAZY_NOTIFIER_LAZY_NOTIFIER_H_
#define GOSSIPSUB_LAZY_NOTIFIER_LAZY_NOTIFIER_H_

#include <omnetpp.h>
#include <set>
#include "../packets_m.h"

using namespace omnetpp;


class LazyNotifier : public cSimpleModule {
  private:
    double notification_interval;

    std::set<int> new_gossip;

    void handleSchedulerMessage(cMessage *schedulerMsg);
    void handleNewGossip(Gossip *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_LAZY_NOTIFIER_LAZY_NOTIFIER_H_
