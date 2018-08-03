#ifndef GOSSIPSUB_TYPE_DISPATCHER_TYPE_DISPATCHER_H_
#define GOSSIPSUB_TYPE_DISPATCHER_TYPE_DISPATCHER_H_

#include <omnetpp.h>

using namespace omnetpp;


class TypeDispatcher : public cSimpleModule {
  private:
    void sendToOutputs(int base_id, int size, cMessage *msg);

  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_TYPE_DISPATCHER_TYPE_DISPATCHER_H_
