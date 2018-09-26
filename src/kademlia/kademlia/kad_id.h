#ifndef KADEMLIA_KADEMLIA_KAD_ID_H_
#define KADEMLIA_KADEMLIA_KAD_ID_H_

#include <vector>
#include <list>
#include <set>
#include <bitset>
#include "constants.h"


class KadId {
  public:
    std::bitset<NUM_BUCKETS> bits;

    KadId();
    KadId(int seed);

    bool operator==(const KadId &other) const;
    bool operator<(const KadId &other) const;

    std::vector<KadId> getNeighbors(std::set<KadId> kad_id, int count);

    bool isCloser(KadId other, KadId reference);
};

std::ostream& operator<<(std::ostream &out, KadId const& kad_id);


#endif  // KADEMLIA_KADEMLIA_KAD_ID_H_
