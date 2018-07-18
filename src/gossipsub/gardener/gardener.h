#ifndef GOSSIPSUB_GARDENER_H_
#define GOSSIPSUB_GARDENER_H_

#include <omnetpp.h>

using namespace omnetpp;


class Gardener : public cSimpleModule
{
  private:
    std::map<int, std::set<int>> requests;

    void prune(int receiverId);
    void graft(int receiverId);

    void handleGraft(Graft *msg);
    void handlePrune(Prune *msg);
    void handleCacheQueryResponse(CacheQueryResponse *msg);
    void handleControl(GardenerControl *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_GARDENER_H
