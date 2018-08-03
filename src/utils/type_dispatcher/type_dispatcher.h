#ifndef UTILS_TYPE_DISPATCHER_TYPE_DISPATCHER_H_
#define UTILS_TYPE_DISPATCHER_TYPE_DISPATCHER_H_

#include <omnetpp.h>

using namespace omnetpp;


class TypeDispatcher : public cSimpleModule {
  private:
    void sendToOutputs(int base_id, int size, cMessage *msg);

  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_TYPE_DISPATCHER_TYPE_DISPATCHER_H_
