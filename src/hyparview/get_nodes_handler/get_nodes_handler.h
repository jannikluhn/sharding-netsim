#ifndef GOSSIPSUB_MEMBERSHIP_GET_NODES_HANDLER_GET_NODES_HANDLER_H_
#define GOSSIPSUB_MEMBERSHIP_GET_NODES_HANDLER_GET_NODES_HANDLER_H_

#include <omnetpp.h>
#include "../peer_list/peer_list.h"

using namespace omnetpp;


class GetNodesHandler : public cSimpleModule {
  private:
    PeerList *peer_list;

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MEMBERSHIP_GET_NODES_HANDLER_GET_NODES_HANDLER_H_
