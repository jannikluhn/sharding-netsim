#ifndef GOSSIPSUB_MEMBERSHIP_PEER_LIST_PEER_LIST_H_
#define GOSSIPSUB_MEMBERSHIP_PEER_LIST_PEER_LIST_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;


class PeerList : public cSimpleModule {
  private:
    int node_id;

    std::vector<int> active_peers;
    std::vector<int> passive_peers;

    std::vector<int> shuffle(std::vector<int> v);

  protected:
    virtual void initialize();

  public:
    void addActivePeer(int peer_id);
    void addPassivePeer(int peer_id);

    void activatePeer(int peer_id);
    void passivatePeer(int peer_id);

    void dropRandomPassivePeer();

    bool isPeer(int peer_id);
    bool isActive(int peer_id);
    bool isPassive(int peer_id);

    int getPeerListSize();
    int getActiveListSize();
    int getPassiveListSize();

    int getPeerByIndex(int index);
    int getActivePeerByIndex(int index);
    int getPassivePeerByIndex(int index);

    int getRandomPeer();
    int getRandomActivePeer();
    int getRandomPassivePeer();

    std::vector<int> getActiveListShuffling();
    std::vector<int> getPassiveListShuffling();
};


#endif  // GOSSIPSUB_MEMBERSHIP_PEER_LIST_PEER_LIST_H_
