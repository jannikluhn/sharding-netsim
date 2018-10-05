#include "node_manager.h"
#include "../hub/hub.h"
#include "../source/source.h"
#include <omnetpp.h>
#include <algorithm>

using namespace omnetpp;


Define_Module(NodeManager);


void NodeManager::initialize() {
    const char *hub_path = par("hubPath").stringValue();
    hub = check_and_cast<Hub *>(getModuleByPath(hub_path));

    const char *source_path = par("sourcePath").stringValue();
    source = check_and_cast<Source *>(getModuleByPath(source_path));

    bootstrap_node_count = par("bootstrapNodeCount").intValue();
    node_count = par("nodeCount").intValue();
    mean_lifetime = par("meanLifetime").doubleValue();

    next_node_id = 0;

    // create bootstrap nodes
    EV_DEBUG << "starting " << bootstrap_node_count << " bootstrap nodes" << endl;
    for (int i = 0; i < bootstrap_node_count; i++) {
        createNode(SimTime::getMaxTime());
    }

    // create other nodes
    for (int i = bootstrap_node_count; i < node_count; i++) {
        createNode(simTime() + exponential(mean_lifetime));
    }

    scheduleAt(nodes.front()->par("timeOfDeath"), new cMessage());
}


void NodeManager::handleMessage(cMessage *msg) {
    // quit next node
    cModule *node = nodes.front();
    int node_id = node->par("nodeId").intValue();
    queues[node_id]->deleteModule();
    EV_DEBUG << "killing node " << node_id << endl;
    hub->deregisterNode(node_id);
    node->deleteModule();

    // start another one
    createNode(simTime() + exponential(mean_lifetime));

    // schedule next node replacement
    scheduleAt(nodes.front()->par("timeOfDeath"), msg);
}


void NodeManager::createNode(simtime_t time_of_death) {
    bool system_module_initialized = getSystemModule()->initialized();
    int node_id = next_node_id;

    EV_DEBUG << "starting node " << node_id << " at " << simTime() << " (lives until "
        << time_of_death << ")" << endl;

    // choose latency and bandwidth (see arXiv:1801.03998 [cs.CR])
    double quality = uniform(0, 1);
    double millisecond = 0.001;
    int megabit = 1024 * 1024;
    double relative_quality;
    double latency;
    double bandwidth;
    if (quality <= 0.1) {
        relative_quality = 0;
        bandwidth = 3.4 * megabit;
        latency = 92 * millisecond;
    } else if (quality <= 0.33) {
        relative_quality = (quality - 0.1) / (0.33 - 0.1);
        bandwidth = (3.4 + (11.2 - 3.4) * relative_quality) * megabit;
        latency = (92 + (125 - 92) * relative_quality) * millisecond;
    } else if (quality <= 0.5) {
        relative_quality = (quality - 0.33) / (0.5 - 0.33);
        bandwidth = (11.2 + (29.4 - 11.2) * relative_quality) * megabit;
        latency = (125 + (152 - 125) * relative_quality) * millisecond;
    } else if (quality <= 0.67) {
        relative_quality = (quality - 0.5) / (0.67 - 0.5);
        bandwidth = (29.4 + (68.3 - 29.4) * relative_quality) * megabit;
        latency = (152 + (200 - 152) * relative_quality) * millisecond;
    } else if (quality <= 0.90) {
        relative_quality = (quality - 0.67) / (0.9 - 0.67);
        bandwidth = (68.3 + (144.4 - 68.3) * relative_quality) * megabit;
        latency = (200 + (276 - 200) * relative_quality) * millisecond;
    } else {
        relative_quality = 0;
        bandwidth = 144.4 * megabit;
        latency = 276 * relative_quality * millisecond;
    }

    // create node and queue
    cModule *node = cModuleType::get("sharding.Node")->create(
        "nodes",
        getParentModule(),
        next_node_id + 1,
        node_id
    );
    node->par("nodeId") = node_id;
    node->par("datarate") = bandwidth;
    node->par("timeOfDeath") = time_of_death.dbl();
    node->finalizeParameters();
    node->buildInside();

    // insert sorted
    nodes.insert(std::lower_bound(
        nodes.begin(),
        nodes.end(),
        node,
        [](const cModule *a, const cModule *b) -> bool {
            return a->par("timeOfDeath").doubleValue() < b->par("timeOfDeath").doubleValue();
        }
    ), node);

    cModule *queue = cModuleType::get("sharding.utils.queue.Queue")->create(
        "queues",
        getParentModule(),
        next_node_id + 1,
        node_id
    );
    queue->finalizeParameters();
    queue->buildInside();

    queues[node_id] = queue;


    // connect node to hub via queue
    cGate *node_out = node->gate("port$o");
    cGate *node_in = node->gate("port$i");
    cGate *queue_in = queue->gate("in");
    cGate *queue_out = queue->gate("out");
    cGate *hub_out, *hub_in;
    hub->getOrCreateFirstUnconnectedGatePair("ports", false, true, hub_in, hub_out);

    cDatarateChannel *node_to_hub = cDatarateChannel::create("channel");
    node_to_hub->setDelay(latency / 2);
    node_to_hub->setDatarate(bandwidth);  // limited upload
    node_out->connectTo(hub_in, node_to_hub);

    cIdealChannel *hub_to_queue = cIdealChannel::create("channel");
    hub_out->connectTo(queue_in, hub_to_queue);

    cDatarateChannel *queue_to_node = cDatarateChannel::create("channel");
    queue_to_node->setDelay(latency / 2);
    queue_to_node->setDatarate(0);  // infinite download
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

    next_node_id++;
    if (system_module_initialized) {
        node->callInitialize();
        queue->callInitialize();
    }
}
