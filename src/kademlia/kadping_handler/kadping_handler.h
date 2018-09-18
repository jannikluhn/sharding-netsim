#ifndef KADEMLIA_KADPING_HANDLER_KADPING_HANDLER_H_
#define KADEMLIA_KADPING_HANDLER_KADPING_HANDLER_H_

#include <omnetpp.h>
#include "../kademlia_peer_table/kademlia_peer_table.h"

using namespace omnetpp;


class KadpingHandler : public cSimpleModule {
  private:
      KademliaPeerTable *peer_table;

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // KADEMLIA_KADPING_HANDLER_KADPING_HANDLER_H_
