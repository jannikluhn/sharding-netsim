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

    bool operator== (const KadId &other);
    std::bitset<256> get_bits();

//    std::bitset<256> get_bits() {
 //               
  //  }
};


class KademliaPeerTable : public cSimpleModule {
  private:
    KadId home_id;

    std::vector<std::list<KadId>> buckets;

    int get_bucket_index(int node_id, int shard_id);

  protected:
    void initialize();

  public:
    void insert(int node_id, int shard_id);
    bool insert_possible(int node_id, int shard_id);
    void update(int node_id, int shard_id);
    void remove(int node_id, int shard_id);
    bool contains(int node_id, int shard_id);
    int size();
};


#endif  // KADEMLIA_KADEMLIA_PEER_TABLE_KADEMLIA_PEER_TABLE_H_
