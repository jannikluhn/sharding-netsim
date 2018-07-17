#ifndef GOSSIPSUB_MULTICASTER_H_
#define GOSSIPSUB_MULTICASTER_H_

#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"

using namespace omnetpp;


class Multicaster : public cSimpleModule
{
  private:
    void handleMulticast(AddressedPacket *msg);
    void handleControl(MulticastControl *msg);
    std::set<int> receiverIds;

  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MULTICASTER_H
