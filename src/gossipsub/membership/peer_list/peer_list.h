#ifndef GOSSIPSUB_MEMBERSHIP_PEER_LIST_PEER_LIST_H_
#define GOSSIPSUB_MEMBERSHIP_PEER_LIST_PEER_LIST_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;


class PeerList : public cSimpleModule {
  private:
    std::vector<int> active_peers;
    std::vector<int> passive_peers;

  protected:
    virtual void initialize();

  public:
    void addActivePeer(int peer_id);
    void addPassivePeer(int peer_id);

    void activatePeer(int peer_id);
    void passivatePeer(int peer_id);

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
};


#endif  // GOSSIPSUB_MEMBERSHIP_PEER_LIST_PEER_LIST_H_
