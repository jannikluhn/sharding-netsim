#ifndef GOSSIPSUB_OVERLAY_MANAGER_OVERLAY_MANAGER_H_
#define GOSSIPSUB_OVERLAY_MANAGER_OVERLAY_MANAGER_H_

#include <omnetpp.h>
#include "../../hyparview/internal_messages_m.h"
#include "../gossipsub_packets_m.h"

using namespace omnetpp;


class OverlayManager : public cSimpleModule {
  private:

    double heartbeat_interval;
    int target_mesh_degree;
    int low_watermark;
    int high_watermark;

    void handleHeartbeat(cMessage *msg);
    void handlePeerListChange(PeerListChange *peer_list_change);
    void handleGraft(Graft *graft);
    void handlePrune(Prune *prune);

    bool isPeer(int node_id);
    bool isMeshPeer(int node_id);
    bool isNonMeshPeer(int node_id);

    std::vector<int> shuffle(std::vector<int> v);

    simsignal_t overlay_changed_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    std::vector<int> mesh_peers;
    std::vector<int> non_mesh_peers;

    std::vector<int> getNonMeshPeerShuffling();
};


#endif  // GOSSIPSUB_OVERLAY_MANAGER_OVERLAY_MANAGER_H_
