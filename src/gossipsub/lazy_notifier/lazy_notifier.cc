#include <omnetpp.h>
#include "../packets_m.h"
#include "lazy_notifier.h"

using namespace omnetpp;


Define_Module(LazyNotifier);

void LazyNotifier::initialize() {
    notification_interval = par("notificationInterval").doubleValue();

    cMessage *scheduler_msg = new cMessage();
    scheduleAt(simTime() + uniform(0, notification_interval), scheduler_msg);
}

void LazyNotifier::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSchedulerMessage(msg);
        // don't delete scheduler msg here as it's reused
    } else if (msg->arrivedOn("gossipInputs")) {
        handleNewGossip(check_and_cast<Gossip *>(msg));
        delete msg;
    } else {
        error("unhandled message");
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
        EV_DEBUG << "notifying about " << new_gossip.size() << " gossip messages" << endl;

        IHave *i_have = new IHave();
        i_have->setContentIdsArraySize(new_gossip.size());
        int i = 0;
        for (auto content_id : new_gossip) {
            i_have->setContentIds(i, content_id);
            i++;
        }

        new_gossip.clear();

        send(i_have, "notificationOutput");
    }

    scheduleAt(simTime() + notification_interval, scheduler_msg);
}
