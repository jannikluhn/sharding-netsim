#ifndef EPISUB_PEER_TRACKER_PEER_TRACKER_H_
#define EPISUB_PEER_TRACKER_PEER_TRACKER_H_

#include <omnetpp.h>
#include <set>

using namespace omnetpp;


class PeerTracker : public cSimpleModule {
  protected:
    void handleMessage(cMessage *msg);

  public:
    std::set<int> eager_peers;
    std::set<int> lazy_peers;

    void addEager(int node_id);
    void makeEager(int node_id);
    void makeLazy(int node_id);
    void remove(int node_id);
    bool isPeer(int node_id);
};


#endif  // EPISUB_PEER_TRACKER_PEER_TRACKER_H_
