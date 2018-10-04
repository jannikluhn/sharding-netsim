#include "peer_table.h"
#include <omnetpp.h>
#include <algorithm>

using namespace omnetpp;


Define_Module(KademliaPeerTable);


void KademliaPeerTable::initialize() {
    int r = intuniform(0, std::numeric_limits<int>::max() - 1);
    setHomeId(KadId(r));

    buckets.resize(NUM_BUCKETS);
    replacements.resize(NUM_BUCKETS);

    peer_list_update_signal = registerSignal("peerListUpdate");
    emit(peer_list_update_signal, 0);
}

void KademliaPeerTable::setHomeId(KadId kad_id) {
    home_id = kad_id;
}

KadId KademliaPeerTable::getHomeId() {
    return home_id;
}

int KademliaPeerTable::getBucketIndex(KadId kad_id) {
    Enter_Method_Silent();

    std::bitset<NUM_BUCKETS> b = kad_id.bits ^ home_id.bits;

    // find first nonzero bit
    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (b[i]) {
            return NUM_BUCKETS - 1 - i;
        }
    }

    error("unreachable (home_id == kad_id)");
    return 0;  // suppress compiler warning
}

void KademliaPeerTable::insert(KadId kad_id, int node_id) {
    Enter_Method_Silent();

    if (contains(kad_id)) {
        error("already in peer table");
    }
    if (!insertPossible(kad_id)) {
        error("bucket full");
    }

    int bucket_index = getBucketIndex(kad_id);
    buckets[bucket_index].push_back(kad_id);

    node_ids[kad_id] = node_id;

    emit(peer_list_update_signal, size());
}

void KademliaPeerTable::insertReplacement(KadId kad_id, int node_id) {
    Enter_Method_Silent();

    int bucket_index = getBucketIndex(kad_id);
    std::tuple<KadId, int> replacement = std::make_tuple(kad_id, node_id);
    std::list<std::tuple<KadId, int>> bucket_replacements = replacements[bucket_index];

    // insert if new
    if (std::find(
        bucket_replacements.begin(),
        bucket_replacements.end(),
        replacement
    ) == bucket_replacements.end()) {
        // remove old elements if necessary
        while (bucket_replacements.size() > REPLACEMENT_LIST_SIZE) {
            bucket_replacements.pop_front();
        }

        bucket_replacements.push_back(replacement);
    }
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
    node_ids.erase(kad_id);

    if (replacements[bucket_index].size() > 0) {
        std::tuple<KadId, int> replacement = replacements[bucket_index].back();
        insert(std::get<0>(replacement), std::get<1>(replacement));
    }

    emit(peer_list_update_signal, size());
}

void KademliaPeerTable::update(KadId kad_id) {
    Enter_Method_Silent();

    if (!contains(kad_id)) {
        error("not in peer table");
    }

    int bucket_index = getBucketIndex(kad_id);
    buckets[bucket_index].remove(kad_id);
    buckets[bucket_index].push_back(kad_id);
}

void KademliaPeerTable::updateIfKnown(KadId kad_id) {
    Enter_Method_Silent();

    if (contains(kad_id)) {
        update(kad_id);
    }
}


int KademliaPeerTable::getNodeId(KadId kad_id) {
    if (!contains(kad_id)) {
        error("not in peer table");
    }
    return node_ids[kad_id];
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

std::vector<int> KademliaPeerTable::bucketSizes() {
    Enter_Method_Silent();

    std::vector<int> sizes;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        sizes.push_back(buckets[i].size());
    }
    return sizes;
}

std::vector<KadId> KademliaPeerTable::getClosestPeers(KadId kad_id, int count) {
    Enter_Method_Silent();

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
