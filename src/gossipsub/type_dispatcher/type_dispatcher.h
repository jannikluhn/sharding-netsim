#ifndef GOSSIPSUB_TYPEDISPATCHER_H_
#define GOSSIPSUB_TYPEDISPATCHER_H_

#include <omnetpp.h>

using namespace omnetpp;


class TypeDispatcher : public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_TYPEDISPATCHER_H
