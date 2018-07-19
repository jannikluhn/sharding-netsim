#ifndef GOSSIPSUB_RECEIVER_DISPATCHER_RECEIVER_DISPATCHER_H_
#define GOSSIPSUB_RECEIVER_DISPATCHER_RECEIVER_DISPATCHER_H_

#include <omnetpp.h>
#include <map>

using namespace omnetpp;


class ReceiverDispatcher : public cSimpleModule {
  private:
    int node_id;
    std::map<int, int> receiver_to_gate_map;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_RECEIVER_DISPATCHER_RECEIVER_DISPATCHER_H_
