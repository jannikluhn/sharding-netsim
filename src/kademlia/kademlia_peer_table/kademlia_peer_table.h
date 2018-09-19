#ifndef KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_
#define KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_

#include <omnetpp.h>
#include <vector>
#include <list>
#include <bitset>

using namespace omnetpp;


const int BUCKET_SIZE = 16;
const int NUM_BUCKETS = 256;


struct KadId {
    int node_id;
    int shard_id;

    bool operator== (const KadId &other) const;
    std::bitset<256> get_bits() const;
};


class KademliaPeerTable : public cSimpleModule {
  private:
    KadId home_id;

    std::vector<std::list<KadId>> buckets;

    int getBucketIndex(KadId kad_id);

  protected:
    void initialize();

  public:
    void insert(KadId kad_id);
    void update(KadId kad_id);
    void updateIfKnown(KadId kad_id);
    void remove(KadId kad_id);

    bool contains(KadId kad_id);
    bool insertPossible(KadId kad_id);
    std::vector<KadId> getNeighbors(KadId kad_id);
    int size();
};


#endif  // KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_
