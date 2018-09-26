#ifndef KADEMLIA_KAD_FIND_NODE_HANDLER_KAD_FIND_NODE_HANDLER_H_
#define KADEMLIA_KAD_FIND_NODE_HANDLER_KAD_FIND_NODE_HANDLER_H_

#include "../kademlia/peer_table.h"
#include <omnetpp.h>

using namespace omnetpp;


class KadFindNodeHandler : public cSimpleModule {
  private:
    KademliaPeerTable *peer_table;

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // KADEMLIA_KAD_FIND_NODE_HANDLER_KAD_FIND_NODE_HANDLER_H_
