#include <omnetpp.h>
#include "../packets_m.h"
#include "lazy_notifier.h"

using namespace omnetpp;


Define_Module(LazyNotifier);

void LazyNotifier::initialize() {
    notification_interval = 1.0;

    cMessage *scheduler_msg = new cMessage();
    scheduleAt(simTime() + notification_interval, scheduler_msg);
}

void LazyNotifier::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSchedulerMessage(msg);
        // don't delete scheduler msg here as it's reused
    } else if (msg->arrivedOn("gossipInput")) {
        handleNewGossip(check_and_cast<Gossip *>(msg));
        delete msg;
    } else {
        EV_ERROR << "unhandled message\n";
        delete msg;
    }
}

void LazyNotifier::handleNewGossip(Gossip *msg) {
    for (int i = 0; i < msg->getContentIdsArraySize(); i++) {
        new_gossip.insert(msg->getContentIds(i));
    }
}

void LazyNotifier::handleSchedulerMessage(cMessage *scheduler_msg) {
    if (!new_gossip.empty()) {
        IHave *i_have = new IHave();
        i_have->setContentIdsArraySize(new_gossip.size());
        int i = 0;
        for (auto content_id : new_gossip) {
            i_have->setContentIds(i, content_id);
        }

        new_gossip.clear();

        send(i_have, "notificationOutput");
    }

    scheduleAt(simTime() + notification_interval, scheduler_msg);
}
