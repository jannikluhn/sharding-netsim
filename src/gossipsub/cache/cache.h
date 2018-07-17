#ifndef GOSSIPSUB_CACHE_H_
#define GOSSIPSUB_CACHE_H_

#include <omnetpp.h>

using namespace omnetpp;


class Cache : public cSimpleModule
{
  private:
      std::set<int> contentIds;
      void handleAddGossip(Gossip *msg);
      void handleQuery(CacheQuery *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_CACHE_H
