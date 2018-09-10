#include <omnetpp.h>
#include "../../packets_m.h"
#include "cache.h"

using namespace omnetpp;


Define_Module(Cache);


void Cache::insert(int content_id, simtime_t creation_time, int bit_length) {
    Enter_Method_Silent();
    if (contains(content_id)) {
        error("tried to insert content twice");
    }
    if (bit_length == 0) {
        error("empty message");
    }
    content_ids.insert(content_id);
    creation_timestamps[content_id] = creation_time;
    bit_lengths[content_id] = bit_length;
}

bool Cache::contains(int content_id) {
    Enter_Method_Silent();
    return content_ids.count(content_id) > 0;
}

simtime_t Cache::getCreationTime(int content_id) {
    Enter_Method_Silent();
    if (!contains(content_id)) {
        error("Unknown content id");
    }
    return creation_timestamps[content_id];
}

int Cache::getBitLength(int content_id) {
    Enter_Method_Silent();
    if (!contains(content_id)) {
        error("Unknown content id");
    }
    return bit_lengths[content_id];
}
