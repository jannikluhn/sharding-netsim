#ifndef UTILS_FUNNEL_FUNNEL_H_
#define UTILS_FUNNEL_FUNNEL_H_

#include <omnetpp.h>

using namespace omnetpp;


class Funnel : public cSimpleModule {
  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_FUNNEL_FUNNEL_H_
