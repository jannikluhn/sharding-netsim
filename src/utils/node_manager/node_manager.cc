#include <omnetpp.h>
#include "node_manager.h"
#include "../hub/hub.h"

using namespace omnetpp;


Define_Module(NodeManager);


void NodeManager::initialize() {
    const char *hub_path = par("hubPath").stringValue();
    hub = check_and_cast<Hub *>(getModuleByPath(hub_path));

    bootstrap_node_count = par("bootstrapNodeCount").intValue();
    target_node_count = par("targetNodeCount").intValue();
    ramp_up_time = par("rampUpTime").doubleValue();

    node_count = 0;
    ramp_up_interval = ramp_up_time / (target_node_count - bootstrap_node_count);


    // create bootstrap nodes
    EV_DEBUG << "starting " << bootstrap_node_count << " bootstrap nodes" << endl;
    for (int i = 0; i < bootstrap_node_count; i++) {
        createNode();
    }

    // start ramp up phase
    EV_DEBUG << "starting " << target_node_count - node_count << " nodes over the next "
        << ramp_up_time << "s" << endl;
    if (node_count < target_node_count) {
        cMessage *scheduler_msg = new cMessage();
        scheduleAt(simTime() + ramp_up_interval, scheduler_msg);
    }
}


void NodeManager::handleMessage(cMessage *scheduler_msg) {
    createNode();

    if (node_count < target_node_count) {
        scheduleAt(simTime() + ramp_up_interval, scheduler_msg);
    } else {
        delete scheduler_msg;
    }
}


void NodeManager::createNode() {
    bool system_module_initialized = getSystemModule()->initialized();
    int node_id = node_count;

    EV_DEBUG << "starting node " << node_id << endl;

    cModule *node = cModuleType::get("sharding.Node")->create(
        "nodes",
        getParentModule(),
        node_count + 1,
        node_id
    );
    node->par("nodeId") = node_id;
    node->finalizeParameters();
    node->buildInside();

    cGate *node_out = node->gate("port$o");
    cGate *node_in = node->gate("port$i");
    cGate *hub_out, *hub_in;
    hub->getOrCreateFirstUnconnectedGatePair("ports", false, true, hub_in, hub_out);

    cDelayChannel *node_out_channel = cDelayChannel::create("channel");
    node_out_channel->setDelay(uniform(0.03 / 2, 0.2 / 2));
    node_out->connectTo(hub_in, node_out_channel);

    cDelayChannel *hub_out_channel = cDelayChannel::create("channel");
    hub_out_channel->setDelay(uniform(0.03 / 2, 0.2 / 2));
    hub_out->connectTo(node_in, hub_out_channel);

    if (!system_module_initialized) {
        node_out_channel->callInitialize();
        hub_out_channel->callInitialize();
    }

    hub->registerNode(node_id, hub_in->getId(), hub_out->getId());

    node_count++;
    if (system_module_initialized) {
        node->callInitialize();
    }
}
