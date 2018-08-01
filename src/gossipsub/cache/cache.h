#ifndef GOSSIPSUB_CACHE_CACHE_H_
#define GOSSIPSUB_CACHE_CACHE_H_

#include <omnetpp.h>
#include <set>

using namespace omnetpp;


class Cache : public cSimpleModule {
  private:
    std::set<int> content_ids;

  public:
    void insert(int content_id);
    bool contains(int content_id);
};


#endif  // GOSSIPSUB_CACHE_CACHE_H_
