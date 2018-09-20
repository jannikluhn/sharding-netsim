#ifndef KADEMLIA_KAD_MANAGER_KAD_MANAGER_H_
#define KADEMLIA_KAD_MANAGER_KAD_MANAGER_H_

#include <omnetpp.h>
#include "../kademlia_peer_table/kademlia_peer_table.h"

using namespace omnetpp;


class KadManager : public cSimpleModule {
  private:
    KademliaPeerTable *peer_table;

    cMessage *self_msg;

    // module parameters
    int node_id;
    int shard_id;
    simtime_t max_lookup_round_duration;
    int lookup_concurrency;
    int max_peers;
    bool hidden;

    // lookup state
    bool lookup_ongoing;
    bool is_last_lookup_round;
    bool is_first_lookup_round;
    KadId lookup_target;
    simtime_t lookup_round_end_time;
    std::set<KadId> queried;
    std::set<KadId> pending_neighbors;
    std::set<KadId> pending_pongs;
    std::set<KadId> candidates;
    KadId last_closest_candidate;

    // message handlers
    void handleSelf(cMessage *msg);
    void handleNeighbors(KadNeighbors *neighbors);
    void handlePong(KadPong *pong);

    // lookup methods
    void lookup(KadId kad_id);
    void startNextLookupRound();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};


#endif  // KADEMLIA_KAD_MANAGER_KAD_MANAGER_H_
