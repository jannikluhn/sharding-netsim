#ifndef GOSSIPSUB_GARDENER_H_
#define GOSSIPSUB_GARDENER_H_

#include <omnetpp.h>

using namespace omnetpp;


class Gardener : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_GARDENER_H
