#ifndef GOSSIPSUB_LAZYNOTIFIER_H_
#define GOSSIPSUB_LAZYNOTIFIER_H_

#include <omnetpp.h>
#include "../packets_m.h"

using namespace omnetpp;


class LazyNotifier : public cSimpleModule
{
  private:
    double notification_interval;

    std::set<int> newGossip;

    void handleSchedulerMessage(cMessage *schedulerMsg);
    void handleNewGossip(Gossip *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_LAZYNOTIFIER_H

