#include <omnetpp.h>
#include "../../packets_m.h"
#include "source.h"

using namespace omnetpp;


Define_Module(Source);


void Source::initialize() {
    gossip_rate = par("gossipRate").doubleValue();
    periodic = par("periodic").boolValue();

    start_time = par("startTime").doubleValue();
    stop_time = par("stopTime").doubleValue();

    gossip_counter = 0;

    new_gossip_emitted_signal = registerSignal("newGossipEmitted");

    simtime_t next_message_time;
    if (periodic) {
        next_message_time = start_time + 1 / gossip_rate;
    } else {
        next_message_time = start_time + exponential(1 / gossip_rate);
    }
    if (next_message_time < stop_time) {
        cMessage *scheduler_msg = new cMessage();
        scheduleAt(next_message_time, scheduler_msg);
    }
}

void Source::handleMessage(cMessage *scheduler_msg) {
    int gate_base_id = gateBaseId("outputs");
    int gate_size = gateSize("outputs");
    if (gate_size == 0) {
        error("no gossipers connected to source");
    }
    int output_gate_id = gate_base_id + intuniform(0, gate_size - 1);

    Gossip *gossip = new Gossip();
    int content_id = gossip_counter;
    gossip_counter++;

    EV_DEBUG << "emitting new gossip with content id " << content_id << endl;

    gossip->setContentIdsArraySize(1);
    gossip->setContentIds(0, content_id);
    gossip->setHops(0);

    send(gossip, output_gate_id);

    emit(new_gossip_emitted_signal, content_id);

    simtime_t next_message_time;
    if (periodic) {
        next_message_time = simTime() + 1 / gossip_rate;
    } else {
        next_message_time = simTime() + exponential(1 / gossip_rate);
    }
    if (next_message_time < stop_time) {
        scheduleAt(next_message_time, scheduler_msg);
    } else {
        delete scheduler_msg;
    }
}
