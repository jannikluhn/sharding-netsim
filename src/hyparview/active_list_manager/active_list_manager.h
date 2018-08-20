#ifndef GOSSIPSUB_MEMBERSHIP_ACTIVE_LIST_MANAGER_ACTIVE_LIST_MANAGER_H_
#define GOSSIPSUB_MEMBERSHIP_ACTIVE_LIST_MANAGER_ACTIVE_LIST_MANAGER_H_

#include <omnetpp.h>
#include "../peer_list/peer_list.h"
#include "../hyparview_packets_m.h"
#include <set>

using namespace omnetpp;


class ActiveListManager : public cSimpleModule {
  private:
    PeerList *peer_list;

    int node_id;
    int num_random_neighbors;
    int num_near_neighbors;
    int num_neighbors;
    double heartbeat_interval;
    int join_ttl;
    int forward_join_ttl;

    std::set<int> neighbor_requests;
    bool is_heart_beating;

    simsignal_t peer_list_update_signal;
    simsignal_t active_list_update_signal;

    void handleHeartbeat(cMessage *msg);
    void handleJoin(Join *join);
    void handleNeighbor(Neighbor *neighbor);
    void handleDisconnect(Disconnect *disconnect);

    void startHeartbeat();
    void sendInitialJoins();
    void acceptNeighborRequest(int node);

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MEMBERSHIP_ACTIVE_LIST_MANAGER_ACTIVE_LIST_MANAGER_H_
