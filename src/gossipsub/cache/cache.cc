#include <omnetpp.h>
#include "../../packets_m.h"
#include "cache.h"

using namespace omnetpp;


Define_Module(Cache);


void Cache::insert(int content_id) {
    content_ids.insert(content_id);
}

bool Cache::contains(int content_id) {
    return content_ids.count(content_id) > 0;
}
