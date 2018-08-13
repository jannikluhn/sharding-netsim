#ifndef GOSSIPSUB_OVERLAY_MANAGER_OVERLAY_MANAGER_H_
#define GOSSIPSUB_OVERLAY_MANAGER_OVERLAY_MANAGER_H_

#include <omnetpp.h>
#include "../../hyparview/internal_messages_m.h"

using namespace omnetpp;


class OverlayManager : public cSimpleModule {
  private:
    std::vector<int> mesh_peers;
    std::vector<int> non_mesh_peers;

    double heartbeat_interval;
    int target_mesh_degree;
    int low_watermark;
    int high_watermark;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void handleHeartbeat(cMessage *msg);
    void handleAddedActivePeer(ActiveListChange *active_list_change);
    void handleRemovedActivePeer(ActiveListChange *active_list_change);
    void handleGraft(Graft *graft);
    void handlePrune(Prune *prune);

    bool isPeer(int node_id);
    bool isMeshPeer(int node_id);
    bool isNonMeshPeer(int node_id);
};


#endif  // GOSSIPSUB_OVERLAY_MANAGER_OVERLAY_MANAGER_H_
