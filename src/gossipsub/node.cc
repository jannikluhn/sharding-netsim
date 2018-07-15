#include <set>
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

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    virtual void handleGossip(Gossip *msg);
    virtual void handleIHave(IHave *msg);
    virtual void handleGraft(Graft *msg);
    virtual void handlePrune(Prune *msg);

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
}

void GossipSub::handleMessage(cMessage *msg)
{
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

void GossipSub::handleGossip(Gossip *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    int msgId = msg->getId();
    bool seen = cache.find(msgId) != cache.end();

    // multicast message
    if (!seen) {
        for (auto gateId : eagerOutGateIds) {
            if (gateId != responseGateId) {
                Gossip *dupMsg = msg->dup();
                send(dupMsg, gateId);
            }
        }
        for (auto gateId : lazyOutGateIds) {
            if (gateId != responseGateId) {
                IHave *iHaveMsg = new IHave();
                iHaveMsg->setId(msgId);
                send(iHaveMsg, gateId);
            }
        }
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

    cache.insert(msgId);
    delete msg;
}

void GossipSub::handleIHave(IHave *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    int msgId = msg->getId();
    bool seen = cache.find(msgId) != cache.end();

    if (!seen) {
        Graft *graftMsg = new Graft();
        graftMsg->setId(msgId);
        send(graftMsg, responseGateId);
    }

    delete msg;
}

void GossipSub::handleGraft(Graft *msg)
{
    int arrivalGateId = msg->getArrivalGateId();
    int responseGateId = getResponseGateId(arrivalGateId);

    int msgId = msg->getId();
    bool seen = cache.find(msgId) != cache.end();

    if (seen) {
        Gossip *gossipMsg = new Gossip();
        gossipMsg->setId(msgId);
        send(gossipMsg, responseGateId);
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
