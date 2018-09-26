#ifndef KADEMLIA_KAD_PING_HANDLER_KAD_PING_HANDLER_H_
#define KADEMLIA_KAD_PING_HANDLER_KAD_PING_HANDLER_H_

#include "../kademlia/peer_table.h"
#include <omnetpp.h>

using namespace omnetpp;


class KadPingHandler : public cSimpleModule {
  private:
    KademliaPeerTable *peer_table;

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // KADEMLIA_KAD_PING_HANDLER_KAD_PING_HANDLER_H_
