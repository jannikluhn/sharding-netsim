#ifndef UTILS_QUEUE_QUEUE_H_
#define UTILS_QUEUE_QUEUE_H_

#include <omnetpp.h>

using namespace omnetpp;


class Queue : public cSimpleModule {
  private:
    cPacketQueue queue;
    cMessage *send_next_packet_msg;
    cChannel *channel;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_QUEUE_QUEUE_H_
