#ifndef UTILS_NODE_MANAGER_NODE_MANAGER_H_
#define UTILS_NODE_MANAGER_NODE_MANAGER_H_

#include <omnetpp.h>
#include "../hub/hub.h"

using namespace omnetpp;


class NodeManager : public cSimpleModule {
  private:
    Hub *hub;
    cModuleType *node_module_type;

    int bootstrap_node_count;
    int target_node_count;
    double ramp_up_time;

    int node_count;
    double ramp_up_interval;

    void createNode();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_NODE_MANAGER_NODE_MANAGER_H_
