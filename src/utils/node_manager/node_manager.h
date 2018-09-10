#ifndef UTILS_NODE_MANAGER_NODE_MANAGER_H_
#define UTILS_NODE_MANAGER_NODE_MANAGER_H_

#include <omnetpp.h>
#include "../hub/hub.h"
#include "../source/source.h"

using namespace omnetpp;


class NodeManager : public cSimpleModule {
  private:
    Hub *hub;
    Source *source;

    std::vector<cModule *> nodes;

    int bootstrap_node_count;
    int target_node_count;
    double ramp_up_time;

    int node_count;
    double ramp_up_interval;

    bool crash;
    simtime_t crash_time;
    double crash_probability;

    cMessage *scheduler_msg;
    cMessage *crash_msg;

    void createNode();
    void crashNode(cModule *node);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};


#endif  // UTILS_NODE_MANAGER_NODE_MANAGER_H_
