#include <omnetpp.h>
#include <algorithm>
#include "kademlia_peer_table.h"
#include <openssl/sha.h>

using namespace omnetpp;


Define_Module(KademliaPeerTable);


void KademliaPeerTable::initialize() {
    int node_id = par("nodeId").intValue();
    int shard_id = par("shardId").intValue();
    home_id = KadId{node_id, shard_id};

    buckets.resize(NUM_BUCKETS);
}

int KademliaPeerTable::getBucketIndex(KadId kad_id) {
    Enter_Method_Silent();

    std::bitset<NUM_BUCKETS> b = kad_id.get_bits();
    b ^= home_id.get_bits();

    // find first nonzero bit
    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (b[i]) {
            return NUM_BUCKETS - 1 - i;
        }
    }

    error("unreachable (home_id == kad_id)");
    return 0;  // suppress compiler warning
}

void KademliaPeerTable::insert(KadId kad_id) {
    Enter_Method_Silent();

    if (contains(kad_id)) {
        error("already in peer table");
    }
    if (!insertPossible(kad_id)) {
        error("bucket full");
    }

    int bucket_index = getBucketIndex(kad_id);
    buckets[bucket_index].push_back(kad_id);
}

bool KademliaPeerTable::insertPossible(KadId kad_id) {
    Enter_Method_Silent();

    if (contains(kad_id)) {
        error("already in peer table");
    }

    int bucket_index = getBucketIndex(kad_id);
    return buckets[bucket_index].size() < BUCKET_SIZE;
}

void KademliaPeerTable::remove(KadId kad_id) {
    Enter_Method_Silent();

    if (!contains(kad_id)) {
        error("not in peer table");
    }

    int bucket_index = getBucketIndex(kad_id);
    buckets[bucket_index].remove(kad_id);
}

void KademliaPeerTable::update(KadId kad_id) {
    Enter_Method_Silent();

    if (!contains(kad_id)) {
        error("not in peer table");
    }

    remove(kad_id);
    insert(kad_id);
}

void KademliaPeerTable::updateIfKnown(KadId kad_id) {
    Enter_Method_Silent();

    if (contains(kad_id)) {
        update(kad_id);
    }
}

bool KademliaPeerTable::contains(KadId kad_id) {
    Enter_Method_Silent();

    int bucket_index = getBucketIndex(kad_id);
    std::list<KadId> bucket = buckets[bucket_index];
    bool found = std::find(bucket.begin(), bucket.end(), kad_id) != bucket.end();
    return found;
}


int KademliaPeerTable::size() {
    Enter_Method_Silent();

    int s = 0;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        s += buckets[i].size();
    }
    return s;
}

std::vector<KadId> KademliaPeerTable::getNeighborsInBuckets(KadId kad_id, int count) {
    int home_bucket_index = getBucketIndex(kad_id);

    // fill candidates with nodes from buckets until we have enough
    std::set<KadId> candidates;
    for (int bucket_index = home_bucket_index; bucket_index >= 0; bucket_index--) {
        if (candidates.size() >= count) {
            break;
        }
        for (auto candidate : buckets[bucket_index]) {
            candidates.insert(candidate);
        }
    }
    for (int bucket_index = home_bucket_index + 1; bucket_index < NUM_BUCKETS; bucket_index++) {
        if (candidates.size() >= count) {
            break;
        }
        for (auto candidate : buckets[bucket_index]) {
            candidates.insert(candidate);
        }
    }
    return kad_id.getNeighbors(candidates, count);
}


//
// KadId
//
bool KadId::operator==(const KadId &other) const {
    return node_id == other.node_id && shard_id == other.shard_id;
}

bool KadId::operator<(const KadId &other) const {
    if (shard_id != other.shard_id) {
        return shard_id < other.shard_id;
    } else {
        return node_id < other.node_id;
    }
}

std::bitset<NUM_BUCKETS> KadId::get_bits() const {
    // hash shard id
    unsigned char shard_input_buffer[sizeof shard_id];
    std::copy(
        static_cast<const unsigned char*>(static_cast<const void*>(&shard_id)),
        static_cast<const unsigned char*>(static_cast<const void*>(&shard_id)) + sizeof shard_id,
        shard_input_buffer
    );
    unsigned char shard_hashed[32];
    SHA256(shard_input_buffer, sizeof shard_id, shard_hashed);

    // hash node id
    unsigned char node_input_buffer[sizeof node_id];
    std::copy(
        static_cast<const unsigned char*>(static_cast<const void*>(&node_id)),
        static_cast<const unsigned char*>(static_cast<const void*>(&node_id)) + sizeof node_id,
        node_input_buffer
    );
    unsigned char node_hashed[32];
    SHA256(node_input_buffer, sizeof node_id, node_hashed);

    // convert to bitset
    std::bitset<NUM_BUCKETS> b;
    // take first 10 bits from hashed shard id
    for (int bit_index = 0; bit_index < 10; bit_index++) {
        char c = shard_hashed[bit_index / 8];
        b[bit_index] = (c & (1 << (bit_index % 8))) == 0;
    }
    // take rest from hashed node id
    for (int bit_index = 10; bit_index < NUM_BUCKETS; bit_index++) {
        char c = node_hashed[bit_index / 8];
        b[bit_index] = c & (1 << (bit_index % 8));
    }
    return b;
}

std::vector<KadId> KadId::getNeighbors(std::set<KadId> nodes, int count) {
    std::vector<KadId> nodes_vec(nodes.begin(), nodes.end());

    // sort candidates by distance to target, closest first
    std::sort(nodes_vec.begin(), nodes_vec.end(), [this](const KadId& lhs, const KadId& rhs) {
        return isCloser(lhs, rhs);
    });

    std::vector<KadId> results;
    for (int i = 0; i < std::min((unsigned long)count, nodes_vec.size()); i++) {
        results.push_back(nodes_vec[i]);
    }
    return results;
}

bool KadId::isCloser(KadId other, KadId reference) {
    std::bitset<NUM_BUCKETS> other_dist = other.get_bits() ^ get_bits();
    std::bitset<NUM_BUCKETS> reference_dist = other.get_bits() ^ get_bits();

    // find first nonzero bit
    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (other_dist[i]) {
            return false;
        } else if (reference_dist[i]) {
            return true;
        }
    }
    return false;
}
