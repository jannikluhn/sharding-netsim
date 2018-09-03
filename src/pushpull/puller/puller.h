#ifndef PUSHPULL_PULLER_PULLER_H_
#define PUSHPULL_PULLER_PULLER_H_

#include <omnetpp.h>
#include "../../utils/cache/cache.h"
#include <vector>
#include <map>

using namespace omnetpp;


class Puller : public cSimpleModule {
  private:
    Cache *cache;
    int gapless_synced_until;

    std::vector<int> peers;
    int pull_fanout;

    simtime_t start_time;
    double period;
    double request_interval;
    double push_time;
    double push_time_delta;

    std::map<cMessage *, int> request_triggers;
    std::map<int, simtime_t> next_request_time;

    void handleSourceGossip(Gossip *gossip);
    void handleExternalGossip(Gossip *gossip);
    void handleRequestTrigger(cMessage *request_trigger);
    void handleHeartbeat(cMessage *heartbeat);
    void handlePeerListChange(PeerListChange *peer_list_change);

    void request(int content_id);
    void insertContentId(int content_id, simtime_t creation_time);

    std::vector<int> shuffle(std::vector<int> v);
    std::vector<int> getPeerShuffling();

    simtime_t getEmissionTime(int content_id);
    int getCurrentContentId();

    simsignal_t new_gossip_received_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // PUSHPULL_PULLER_PULLER_H_
