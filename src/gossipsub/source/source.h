#ifndef GOSSIPSUB_SOURCE_H_
#define GOSSIPSUB_SOURCE_H_

#include <omnetpp.h>

using namespace omnetpp;


class Source : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_SOURCE_H
