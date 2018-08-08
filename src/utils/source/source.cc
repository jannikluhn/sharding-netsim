#include <omnetpp.h>
#include "../../packets_m.h"
#include "source.h"

using namespace omnetpp;


Define_Module(Source);


void Source::initialize() {
    const char *cache_path = par("cachePath").stringValue();
    cache = check_and_cast<Cache *>(getModuleByPath(cache_path));

    active = par("active").boolValue();
    rate = par("rate").doubleValue();
    start_time = par("startTime").doubleValue();
    stop_time = par("stopTime").doubleValue();
    node_id = par("nodeId").intValue();

    new_gossip_emitted_signal = registerSignal("newGossipEmitted");

    if (active) {
        cMessage *scheduler_msg = new cMessage();
        scheduleAt(start_time + exponential(1 / rate), scheduler_msg);
    }
}

void Source::handleMessage(cMessage *scheduler_msg) {
    Gossip *msg = new Gossip();
    int content_id = msg->getTreeId();

    EV_DEBUG << "spawning new gossip with id " << content_id << endl;

    msg->setContentIdsArraySize(1);
    msg->setContentIds(0, msg->getTreeId());
    msg->setSender(node_id);
    msg->setHops(0);

    int gate_base_id = gateBaseId("outputs");
    for (int i = 0; i < gateSize("outputs"); i++) {
        int gate_id = gate_base_id + i;
        send(msg->dup(), gate_id);
    }
    delete msg;

    cache->insert(content_id);
    emit(new_gossip_emitted_signal, content_id);

    simtime_t next_message_time = simTime() + exponential(1 / rate);
    if (stop_time < 0 || next_message_time < stop_time) {
        scheduleAt(next_message_time, scheduler_msg);
    } else {
        delete scheduler_msg;
    }
}
