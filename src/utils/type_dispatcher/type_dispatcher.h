#ifndef UTILS_TYPE_DISPATCHER_TYPE_DISPATCHER_H_
#define UTILS_TYPE_DISPATCHER_TYPE_DISPATCHER_H_

#include <omnetpp.h>

using namespace omnetpp;


class TypeDispatcher : public cSimpleModule {
  private:
    int num_packet_types;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_TYPE_DISPATCHER_TYPE_DISPATCHER_H_
