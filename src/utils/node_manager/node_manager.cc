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
    crash = par("crash").boolValue();
    crash_time = par("crashTime").doubleValue();
    crash_probability = par("crashProbability").doubleValue();

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
    scheduler_msg = new cMessage();
    if (node_count < target_node_count) {
        cMessage *scheduler_msg = new cMessage();
        scheduleAt(simTime() + ramp_up_interval, scheduler_msg);
    }

    // crash
    crash_msg = new cMessage();
    scheduleAt(crash_time, crash_msg);
}


void NodeManager::handleMessage(cMessage *msg) {
    if (msg == scheduler_msg) {
        createNode();

        if (node_count < target_node_count) {
            scheduleAt(simTime() + ramp_up_interval, scheduler_msg);
        } else {
            delete scheduler_msg;
        }
    } else if (msg == crash_msg) {
        if (crash) {
            for (int i = 0; i < nodes.size(); i++) {
                if (uniform(0, 1) < crash_probability) {
                    crashNode(nodes[i]);
                }
            }
        }
    }
}


void NodeManager::createNode() {
    bool system_module_initialized = getSystemModule()->initialized();
    int node_id = node_count;

    EV_DEBUG << "starting node " << node_id << endl;

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
        node_count + 1,
        node_id
    );
    node->par("nodeId") = node_id;
    node->par("datarate") = bandwidth;
    node->finalizeParameters();
    node->buildInside();
    nodes.push_back(node);

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

    node_count++;
    if (system_module_initialized) {
        node->callInitialize();
        queue->callInitialize();
    }
}

void NodeManager::crashNode(cModule *node) {
    node->par("crashed").setBoolValue(true) ;
}

void NodeManager::finish() {
    cancelAndDelete(scheduler_msg);
    cancelAndDelete(crash_msg);
}
