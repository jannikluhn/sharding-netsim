#include <omnetpp.h>
#include <algorithm>
#include "kademlia_peer_table.h"
#include <openssl/sha.h>

using namespace omnetpp;


Define_Module(KademliaPeerTable);


void KademliaPeerTable::initialize() {
    buckets.resize(NUM_BUCKETS);

    int node_id = par("nodeId");
    int shard_id = par("shardId");
    home_id = KadId{node_id, shard_id};
}

int KademliaPeerTable::get_bucket_index(int node_id, int shard_id) {
    KadId other_id = {node_id, shard_id};
    std::bitset<256> b = other_id.get_bits();
    b ^= home_id.get_bits();

    // find first nonzero bit
    for (int i = 0; i < 256; i++) {
        if (b[i]) {
            return i;
        }
    }

    error("unreachable");
    return 0;  // supress compiler warning
}

void KademliaPeerTable::insert(int node_id, int shard_id) {
    if (contains(node_id, shard_id)) {
        error("already in peer table");
    }
    if (insert_possible(node_id, shard_id)) {
        error("bucket full");
    }

    KadId kad_id = {node_id, shard_id};
    int bucket_index = get_bucket_index(node_id, shard_id);
    buckets[bucket_index].push_back(kad_id);
}

bool KademliaPeerTable::insert_possible(int node_id, int shard_id) {
    if (contains(node_id, shard_id)) {
        error("already in peer table");
    }

    int bucket_index = get_bucket_index(node_id, shard_id);
    return buckets[bucket_index].size() >= BUCKET_SIZE;
}

void KademliaPeerTable::remove(int node_id, int shard_id) {
    if (!contains(node_id, shard_id)) {
        error("not in peer table");
    }

    KadId kad_id = {node_id, shard_id};
    int bucket_index = get_bucket_index(node_id, shard_id);
    buckets[bucket_index].remove(kad_id);
}

void KademliaPeerTable::update(int node_id, int shard_id) {
    if (!contains(node_id, shard_id)) {
        error("not in peer table");
    }

    remove(node_id, shard_id);
    insert(node_id, shard_id);
}

void KademliaPeerTable::updateIfKnown(int node_id, int shard_id) {
    if (contains(node_id, shard_id)) {
        update(node_id, shard_id);
    }
}

bool KademliaPeerTable::contains(int node_id, int shard_id) {
    int bucket_index = get_bucket_index(node_id, shard_id);
    KadId kad_id = {node_id, shard_id};
    std::list<KadId> bucket = buckets[bucket_index];
    bool found = std::find(bucket.begin(), bucket.end(), kad_id) != bucket.end();
    return found;
}


int KademliaPeerTable::size() {
    int s = 0;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        s += buckets[i].size();
    }
    return s;
}


//
// KadId
//
bool KadId::operator== (const KadId &other) {
    return node_id == other.node_id && shard_id == other.shard_id;
}

std::bitset<256> KadId::get_bits() {
    // hash shard id
    unsigned char shard_input_buffer[sizeof shard_id];
    std::copy(
        static_cast<const unsigned char*>(static_cast<const void*>(&shard_id)),
        static_cast<const unsigned char*>(static_cast<const void*>(&shard_id)) + sizeof shard_id,
        shard_input_buffer
    );
    unsigned char shard_hashed[20];
    SHA1(shard_input_buffer, sizeof shard_id, shard_hashed);

    // hash node id
    unsigned char node_input_buffer[sizeof node_id];
    std::copy(
        static_cast<const unsigned char*>(static_cast<const void*>(&node_id)),
        static_cast<const unsigned char*>(static_cast<const void*>(&node_id)) + sizeof node_id,
        node_input_buffer
    );
    unsigned char node_hashed[20];
    SHA1(node_input_buffer, sizeof node_id, node_hashed);

    // convert to bitset
    std::bitset<256> b;
    // take first 10 bits from hashed shard id
    for (int bit_index = 0; bit_index < 10; bit_index++) {
        char c = shard_hashed[bit_index / 8];
        b[bit_index] = c & (1 << (bit_index % 8));
    }
    // take rest from hashed node id
    for (int bit_index = 10; bit_index < 256; bit_index++) {
        char c = node_hashed[bit_index / 8];
        b[bit_index] = c & (1 << (bit_index % 8));
    }
    return b;
}
