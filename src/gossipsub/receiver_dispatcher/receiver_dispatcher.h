#ifndef GOSSIPSUB_RECEIVERDISPATCHER_H_
#define GOSSIPSUB_RECEIVERDISPATCHER_H_

#include <omnetpp.h>

using namespace omnetpp;


class ReceiverDispatcher : public cSimpleModule
{
  private:
    int nodeId;
    std::map<int, int> receiverToGateMap;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_RECEIVERDISPATCHER_H
