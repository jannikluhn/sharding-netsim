#ifndef KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_
#define KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_

#include "kad_id.h"
#include <omnetpp.h>

using namespace omnetpp;


class KademliaPeerTable : public cSimpleModule {
  private:
    KadId home_id;

    std::vector<std::list<KadId>> buckets;
    std::map<KadId, int> node_ids;

    int getBucketIndex(KadId kad_id);

  protected:
    void initialize();

  public:
    KadId getHomeId();
    void setHomeId(KadId kad_id);

    void insert(KadId kad_id, int node_id);
    void update(KadId kad_id);
    void updateIfKnown(KadId kad_id);
    void remove(KadId kad_id);

    int getNodeId(KadId kad_id);
    bool contains(KadId kad_id);
    bool insertPossible(KadId kad_id);
    std::vector<KadId> getClosestPeers(KadId kad_id, int count);

    int size();
    std::vector<int> bucketSizes();
};


#endif  // KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_
