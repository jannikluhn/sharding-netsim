#ifndef GOSSIPSUB_FUNNEL_FUNNEL_H_
#define GOSSIPSUB_FUNNEL_FUNNEL_H_

#include <omnetpp.h>

using namespace omnetpp;


class Funnel : public cSimpleModule {
  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_FUNNEL_FUNNEL_H_
