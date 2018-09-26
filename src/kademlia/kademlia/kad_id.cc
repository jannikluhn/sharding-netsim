#include "kad_id.h"
#include <openssl/sha.h>
#include <algorithm>
#include <omnetpp.h>


//
// KadId
//

KadId::KadId() {
    bits = std::bitset<NUM_BUCKETS>();
}

KadId::KadId(int seed) {
    // hash random number
    unsigned char input_buffer[sizeof seed];
    std::copy(
        static_cast<const unsigned char*>(static_cast<const void*>(&seed)),
        static_cast<const unsigned char*>(static_cast<const void*>(&seed)) + sizeof seed,
        input_buffer
    );
    unsigned char output_buffer[32];
    SHA256(input_buffer, sizeof seed, output_buffer);

    // convert to bitset
    std::bitset<NUM_BUCKETS> b;
    for (int bit_index = 0; bit_index < NUM_BUCKETS; bit_index++) {
        char c = output_buffer[bit_index / 8];
        b[bit_index] = (c & (1 << (bit_index % 8))) == 0;
    }

    bits = b;
}

bool KadId::operator==(const KadId &other) const {
    return bits == other.bits;
}

bool KadId::operator<(const KadId &other) const {
    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (bits[i] && !other.bits[i]) {
            return false;
        } else if (other.bits[i] && !bits[i]) {
            return true;
        }
    }
    return false;  // equal
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
    std::bitset<NUM_BUCKETS> other_dist = other.bits ^ bits;
    std::bitset<NUM_BUCKETS> reference_dist = reference.bits ^ bits;

    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (other_dist[i] && !reference_dist[i]) {
            return false;
        } else if (reference_dist[i] && !other_dist[i]) {
            return true;
        }
    }
    return false;  // equal
}


std::ostream& operator<<(std::ostream &out, KadId const& kad_id) {
    for (int i = 0; i < 3; i++) {
        out << kad_id.bits[i];
    }
    out << "...";
    for (int i = NUM_BUCKETS - 3; i < NUM_BUCKETS; i++) {
        out << kad_id.bits[i];
    }

    return out;
}
