#include <omnetpp.h>
#include "node_manager.h"
#include "../hub/hub.h"
#include "../source/source.h"

using namespace omnetpp;


Define_Module(NodeManager);


void NodeManager::initialize() {
    const char *hub_path = par("hubPath").stringValue();
    hub = check_and_cast<Hub *>(getModuleByPath(hub_path));

    const char *source_path = par("sourcePath").stringValue();
    source = check_and_cast<Source *>(getModuleByPath(source_path));

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

    cModule *queue = cModuleType::get("sharding.utils.queue.Queue")->create(
        "queues",
        getParentModule(),
        node_count + 1,
        node_id
    );
    queue->finalizeParameters();
    queue->buildInside();

    // connect node to hub via queue
    cGate *node_out = node->gate("port$o");
    cGate *node_in = node->gate("port$i");
    cGate *queue_in = queue->gate("in");
    cGate *queue_out = queue->gate("out");
    cGate *hub_out, *hub_in;
    hub->getOrCreateFirstUnconnectedGatePair("ports", false, true, hub_in, hub_out);

    cDatarateChannel *node_to_hub = cDatarateChannel::create("channel");
    node_to_hub->setDelay(uniform(0.03 / 2, 0.2 / 2));
    node_to_hub->setDatarate(30 * 1024 * 1024);  // upload
    node_out->connectTo(hub_in, node_to_hub);

    cIdealChannel *hub_to_queue = cIdealChannel::create("channel");
    hub_out->connectTo(queue_in, hub_to_queue);

    cDatarateChannel *queue_to_node = cDatarateChannel::create("channel");
    queue_to_node->setDelay(uniform(0.03 / 2, 0.2 / 2));
    queue_to_node->setDatarate(3 * 1024 * 1024);  // download
    queue_out->connectTo(node_in, queue_to_node);

    // connect source to node
    cGate *source_in = node->gate("sourceInput");
    cGate *source_out = source->getOrCreateFirstUnconnectedGate("outputs", 0, false, true);
    cIdealChannel *source_to_node = cIdealChannel::create("channel");
    source_out->connectTo(source_in, source_to_node);

    if (!system_module_initialized) {
        node_to_hub->callInitialize();
        hub_to_queue->callInitialize();
        queue_to_node->callInitialize();

        source_to_node->callInitialize();
    }

    hub->registerNode(node_id, hub_in->getId(), hub_out->getId());

    node_count++;
    if (system_module_initialized) {
        node->callInitialize();
        queue->callInitialize();
    }
}
