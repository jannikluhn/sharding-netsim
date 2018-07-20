#include <omnetpp.h>
#include "../packets_m.h"
#include "source.h"

using namespace omnetpp;


Define_Module(Source);


void Source::initialize() {
    rate = par("rate").doubleValue();

    new_gossip_emitted_signal = registerSignal("newGossipEmitted");

    cMessage *scheduler_msg = new cMessage();
    scheduleAt(simTime() + exponential(1 / rate), scheduler_msg);
}

void Source::handleMessage(cMessage *scheduler_msg) {
    Gossip *msg = new Gossip();
    msg->setContentIdsArraySize(1);
    msg->setContentIds(0, msg->getTreeId());
    msg->setSender(getParentModule()->getId());

    send(msg, "out");
    emit(new_gossip_emitted_signal, msg->getContentIds(0));

    scheduleAt(simTime() + exponential(1 / rate), scheduler_msg);
}
