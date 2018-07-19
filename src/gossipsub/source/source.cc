#include <omnetpp.h>
#include "../packets_m.h"
#include "source.h"

using namespace omnetpp;


Define_Module(Source);


void Source::initialize() {
    cMessage *scheduler_msg = new cMessage();
    scheduleAt(simTime() + exponential(1.), scheduler_msg);
}

void Source::handleMessage(cMessage *scheduler_msg) {
    Gossip *msg = new Gossip();
    msg->setContentIdsArraySize(1);
    msg->setContentIds(0, msg->getTreeId());
    msg->setSender(getParentModule()->getId());
    send(msg, "out");

    scheduleAt(simTime() + exponential(1.), scheduler_msg);
}
