#ifndef GOSSIPSUB_MEMBERSHIP_PEER_LIST_MANAGER_PEER_LIST_MANAGER_H_
#define GOSSIPSUB_MEMBERSHIP_PEER_LIST_MANAGER_PEER_LIST_MANAGER_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;


class PeerListManager : public cSimpleModule {
  private:
    std::vector<int> active_peers;
    std::vector<int> passive_peers;

  public:
    void addActivePeer(int peer_id);
    void addPassivePeer(int peer_id);

    void activatePeer(int peer_id);
    void passivatePeer(int peer_id);

    bool isPeer(int peer_id);
    bool isActive(int peer_id);
    bool isPassive(int peer_id);

    int getRandomPeer();
    int getRandomActivePeer();
    int getRandomPassivePeer();
};


#endif  // GOSSIPSUB_MEMBERSHIP_PEER_LIST_MANAGER_PEER_LIST_MANAGER_H_
