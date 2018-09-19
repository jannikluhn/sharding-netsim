#ifndef KADEMLIA_KAD_PING_HANDLER_KAD_PING_HANDLER_H_
#define KADEMLIA_KAD_PING_HANDLER_KAD_PING_HANDLER_H_

#include <omnetpp.h>
#include "../kademlia_peer_table/kademlia_peer_table.h"

using namespace omnetpp;


class KadPingHandler : public cSimpleModule {
  private:
      KademliaPeerTable *peer_table;

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // KADEMLIA_KAD_PING_HANDLER_KAD_PING_HANDLER_H_
