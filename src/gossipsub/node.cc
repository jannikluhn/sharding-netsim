#include <set>
#include <algorithm>
#include <omnetpp.h>

using namespace omnetpp;

#include "messages_m.h"


class GossipSub : public cSimpleModule
{
  private:
    std::set<long> cache;

    int inGateBaseId;
    int outGateBaseId;
    int sourceGateId;

    std::set<int> eagerInGateIds;
    std::set<int> eagerOutGateIds;
    std::set<int> lazyInGateIds;
    std::set<int> lazyOutGateIds;

    std::set<int> lazyNotificationQueue;
    std::set<int> missingMessageQueue;
    double lazyNotificationInterval;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void handleSelf(cMessage *msg);
    void handleGossip(Gossip *msg);
    void handleIHave(IHave *msg);
    void handleGraft(Graft *msg);
    void handlePrune(Prune *msg);

    int getResponseGateId(int arrivalGateId);
};

Define_Module(GossipSub);


void GossipSub::initialize()
{
    inGateBaseId = gateBaseId("ports$i");
    outGateBaseId = gateBaseId("ports$o");
    for (int i = 0; i < gateSize("ports"); i++) {
        eagerInGateIds.insert(inGateBaseId + i);
        eagerOutGateIds.insert(outGateBaseId + i);
    }
    sourceGateId = gateBaseId("source");

    cMessage *schedulerMsg = new cMessage();
    scheduleAt(simTime() + 3, schedulerMsg);
}

void GossipSub::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        return handleSelf(msg);
    }

    Gossip *gossipMsg = dynamic_cast<Gossip *>(msg);
    if (gossipMsg) {
        return handleGossip(gossipMsg);
    }

    IHave *iHaveMsg = dynamic_cast<IHave *>(msg);
    if (iHaveMsg) {
        return handleIHave(iHaveMsg);
    }

    Graft *graftMsg = dynamic_cast<Graft *>(msg);
    if (graftMsg) {
        return handleGraft(graftMsg);
    }

    Prune *pruneMsg = dynamic_cast<Prune *>(msg);
    if (pruneMsg) {
        return handlePrune(pruneMsg);
    }

    EV_ERROR << "Unhandled message " << msg << "\n";
    delete msg;
}

void GossipSub::handleSelf(cMessage *msg)
{
    if (!lazyNotificationQueue.empty()) {
        for (auto gateId : lazyOutGateIds) {
            IHave *msg = new IHave();
            msg->setContentIdsArraySize(lazyNotificationQueue.size());
            int i = 0;
            for (auto contentId : lazyNotificationQueue) {
                msg->setContentIds(i, contentId);
                i++;
            }
            send(msg, gateId);
        }

        lazyNotificationQueue.clear();
    }

    scheduleAt(simTime() + 0.1, msg);
}

void GossipSub::handleGossip(Gossip *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    int contentId = msg->getContentId();
    bool seen = cache.find(contentId) != cache.end();

    if (!seen) {
        // multicast message
        for (auto gateId : eagerOutGateIds) {
            if (gateId != responseGateId) {
                Gossip *dupMsg = msg->dup();
                send(dupMsg, gateId);
            }
        }

        // notify lazy peers later
        lazyNotificationQueue.insert(msg->getContentId());
    }

    // update peer status
    if (arrivalGateId != sourceGateId) {
        if (!seen) {
            eagerOutGateIds.insert(responseGateId);
            lazyOutGateIds.erase(responseGateId);
        } else {
            lazyOutGateIds.insert(responseGateId);
            eagerOutGateIds.erase(responseGateId);

            Prune *pruneMsg = new Prune();
            send(pruneMsg, responseGateId);
        }
    }

    cache.insert(contentId);
    delete msg;
}

void GossipSub::handleIHave(IHave *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    std::set<int> contentIds;;
    std::set<int> missingContentIds;

    for (int i = 0; i < msg->getContentIdsArraySize(); i++) {
        contentIds.insert(msg->getContentIds(i));
    }
    std::set_difference(
        contentIds.begin(),
        contentIds.end(),
        cache.begin(),
        cache.end(),
        std::inserter(missingContentIds, missingContentIds.end())
    );

    if (!missingContentIds.empty()) {
        Graft *graftMsg = new Graft();
        graftMsg->setContentIdsArraySize(missingContentIds.size());
        int i = 0;
        for (auto contentId : missingContentIds) {
            graftMsg->setContentIds(i, contentId);
        }
        send(graftMsg, responseGateId);
    }

    delete msg;
}

void GossipSub::handleGraft(Graft *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    std::set<int> contentIds;;
    std::set<int> missingContentIds;

    for (int i = 0; i < msg->getContentIdsArraySize(); i++) {
        contentIds.insert(msg->getContentIds(i));
    }
    std::set_difference(
        contentIds.begin(),
        contentIds.end(),
        cache.begin(),
        cache.end(),
        std::inserter(missingContentIds, missingContentIds.end())
    );

    if (!missingContentIds.empty()) {
        Gossip *gossipMsg = new Gossip();
        for (auto contentId : missingContentIds) {
            gossipMsg->setContentId(contentId);
            send(gossipMsg, responseGateId);
        }
    }

    eagerOutGateIds.insert(responseGateId);
    lazyOutGateIds.erase(responseGateId);

    delete msg;
}

void GossipSub::handlePrune(Prune *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    lazyOutGateIds.insert(responseGateId);
    eagerOutGateIds.erase(responseGateId);

    delete msg;
}

int GossipSub::getResponseGateId(int arrivalGateId)
{
    return outGateBaseId + (arrivalGateId - inGateBaseId);
}
