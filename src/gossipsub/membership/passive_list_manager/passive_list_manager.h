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

    int shuffle_interval;
    int active_shuffling_size;
    int passive_shuffling_size;
    int shuffle_ttl;

    bool view_initialization_finished;
    std::set<int> pending_getnodes_requests;
    int num_pending_shuffle_requests;

    void startViewInitialization();
    void handleNodes(Nodes *nodes);

    void handleHeartbeat(cMessage *msg);
    void initiateShuffle();
    void handleShuffle(Shuffle *shuffle);
    void handleShuffleReply(ShuffleReply *shuffle_reply);

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MEMBERSHIP_PASSIVE_LIST_MANAGER_PASSIVE_LIST_MANAGER_H_
