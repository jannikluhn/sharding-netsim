#include <omnetpp.h>
#include "../../packets_m.h"
#include "source.h"

using namespace omnetpp;


Define_Module(Source);


void Source::initialize() {
    active = par("active").boolValue();
    gossip_rate = par("gossipRate").doubleValue();
    start_time = par("startTime").doubleValue();
    stop_time = par("stopTime").doubleValue();

    new_gossip_emitted_signal = registerSignal("newGossipEmitted");

    if (active) {
        cMessage *scheduler_msg = new cMessage();
        scheduleAt(start_time + exponential(1 / gossip_rate), scheduler_msg);
    }
}

void Source::handleMessage(cMessage *scheduler_msg) {
    Gossip *msg = new Gossip();
    int content_id = msg->getTreeId();

    EV_DEBUG << "emitting new gossip with content id " << content_id << endl;

    msg->setContentIdsArraySize(1);
    msg->setContentIds(0, msg->getTreeId());
    msg->setHops(0);

    int gate_base_id = gateBaseId("outputs");
    for (int i = 0; i < gateSize("outputs"); i++) {
        int gate_id = gate_base_id + i;
        send(msg->dup(), gate_id);
    }
    delete msg;

    emit(new_gossip_emitted_signal, content_id);

    simtime_t next_message_time = simTime() + exponential(1 / gossip_rate);
    if (stop_time == 0 || next_message_time < stop_time) {
        scheduleAt(next_message_time, scheduler_msg);
    } else {
        delete scheduler_msg;
    }
}
