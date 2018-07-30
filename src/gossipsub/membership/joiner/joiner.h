#ifndef GOSSIPSUB_MEMBERSHIP_JOINER_JOINER_H_
#define GOSSIPSUB_MEMBERSHIP_JOINER_JOINER_H_

#include <omnetpp.h>
#include "../peer_list/peer_list.h"
#include "../../packets_m.h"
#include <set>

using namespace omnetpp;


class Joiner : public cSimpleModule {
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

    void handleHeartbeat(cMessage *msg);
    void handleJoin(Join *join);
    void handleNeighbor(Neighbor *neighbor);
    void handleDisconnect(Disconnect *disconnect);
    void handleForwardJoin(ForwardJoin *forward_join);

    void startHeartbeat();
    void sendInitialJoins();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_MEMBERSHIP_JOINER_JOINER_H_

