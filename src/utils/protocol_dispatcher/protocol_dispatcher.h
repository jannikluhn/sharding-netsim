#ifndef UTILS_PROTOCOL_DISPATCHER_PROTOCOL_DISPATCHER_H_
#define UTILS_PROTOCOL_DISPATCHER_PROTOCOL_DISPATCHER_H_

#include <omnetpp.h>

using namespace omnetpp;


class ProtocolDispatcher : public cSimpleModule {
  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_PROTOCOL_DISPATCHER_PROTOCOL_DISPATCHER_H_
