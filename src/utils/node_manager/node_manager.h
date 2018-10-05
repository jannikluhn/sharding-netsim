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

    std::list<cModule *> nodes;  // ordered by time of death
    std::map<int, cModule *> queues;  // maps node id to corresponding queue module

    int bootstrap_node_count;
    int node_count;
    double mean_lifetime;

    int next_node_id;

//  bool crash;
//  simtime_t crash_time;
//  double crash_probability;

    void createNode(simtime_t time_of_death);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_NODE_MANAGER_NODE_MANAGER_H_
