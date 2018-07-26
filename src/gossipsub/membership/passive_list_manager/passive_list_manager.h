#ifndef GOSSIPSUB_MEMBERSHIP_PASSIVE_LIST_MANAGER_PASSIVE_LIST_MANAGER_H_
#define GOSSIPSUB_MEMBERSHIP_PASSIVE_LIST_MANAGER_PASSIVE_LIST_MANAGER_H_

#include <omnetpp.h>
#include "../peer_list/peer_list.h"
#include "../../packets_m.h"

using namespace omnetpp;


class PassiveListManager : public cSimpleModule {
  private:
    PeerList *peer_list;
    int node_id;

    int c_rand;

    bool view_initialization_finished;
    std::set<int> outstanding_getnodes_requests;

    void startViewInitialization();
    void handleNodes(Nodes *msg);

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MEMBERSHIP_PASSIVE_LIST_MANAGER_PASSIVE_LIST_MANAGER_H_
