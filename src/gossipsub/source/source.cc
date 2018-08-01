#include <omnetpp.h>
#include "../packets_m.h"
#include "source.h"

using namespace omnetpp;


Define_Module(Source);


void Source::initialize() {
    active = par("active").boolValue();
    rate = par("rate").doubleValue();
    start_time = par("startTime").doubleValue();
    node_id = par("nodeId").intValue();

    new_gossip_emitted_signal = registerSignal("newGossipEmitted");

    if (active) {
        cMessage *scheduler_msg = new cMessage();
        scheduleAt(start_time + exponential(1 / rate), scheduler_msg);
    }
}

void Source::handleMessage(cMessage *scheduler_msg) {
    Gossip *msg = new Gossip();
    msg->setContentIdsArraySize(1);
    msg->setContentIds(0, msg->getTreeId());
    msg->setSender(node_id);
    msg->setHops(0);

    send(msg, "out");
    emit(new_gossip_emitted_signal, msg->getContentIds(0));

    scheduleAt(simTime() + exponential(1 / rate), scheduler_msg);
}
