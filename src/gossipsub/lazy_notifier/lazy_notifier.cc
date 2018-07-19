#include <omnetpp.h>
#include "../packets_m.h"
#include "lazy_notifier.h"

using namespace omnetpp;


Define_Module(LazyNotifier);

void LazyNotifier::initialize()
{
    notification_interval = 1.0;

    cMessage *schedulerMsg = new cMessage();
    scheduleAt(simTime() + notification_interval, schedulerMsg);
}

void LazyNotifier::handleMessage(cMessage *msg)
{
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

void LazyNotifier::handleNewGossip(Gossip *msg)
{
    for (int i = 0; i < msg->getContentIdsArraySize(); i++) {
        newGossip.insert(msg->getContentIds(i));
    }
}

void LazyNotifier::handleSchedulerMessage(cMessage *schedulerMsg)
{
    if (!newGossip.empty()) {
        IHave *iHave = new IHave();
        iHave->setContentIdsArraySize(newGossip.size());
        int i = 0;
        for (auto contentId : newGossip) {
            iHave->setContentIds(i, contentId);
        }

        newGossip.clear();

        send(iHave, "notificationOutput");
    }

    scheduleAt(simTime() + notification_interval, schedulerMsg);
}
