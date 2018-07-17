#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "cache.h"

using namespace omnetpp;


Define_Module(Cache);


void Cache::initialize()
{
}

void Cache::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("addGossipInputs")) {
        handleAddGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("queryPorts")) {
        handleQuery(check_and_cast<CacheQuery *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }

    delete msg;
}

void Cache::handleAddGossip(Gossip *msg)
{
    std::set<int> newContentIds;

    // insert content Ids and note which ones are new
    int n = msg->getContentIdsArraySize();
    for (int i = 0; i < n; i++) {
        int contentId = msg->getContentIds(i);
        if (contentIds.find(contentId) == contentIds.end()) {
            // new entry
            contentIds.insert(contentId);
            newContentIds.insert(contentId);
        }
    }

    if (!newContentIds.empty()) {
        // send new gossip to corresponding outputs
        Gossip *newGossipMsg = new Gossip();
        newGossipMsg->setContentIdsArraySize(newContentIds.size());
        int i = 0;
        for (auto contentId : newContentIds) {
            newGossipMsg->setContentIds(i, contentId);
            i++;
        }

        int newGossipBaseGateId = gateBaseId("newGossipOutputs");
        for (int i = 0; i < gateSize("newGossipOutputs"); i++) {
            send(newGossipMsg->dup(), newGossipBaseGateId + i);
        }
        delete newGossipMsg;
    }


    if (msg->getSender() != getParentModule()->getId()) {
        GardenerControl *gardenerControl = new GardenerControl();
        int sender = msg->getSender();
        if (newContentIds.empty()) {
            // make sender lazy
            gardenerControl->setPruneReceiversArraySize(1);
            gardenerControl->setPruneReceivers(0, sender);
        } else {
            // make sender eager
            gardenerControl->setGraftReceiversArraySize(1);
            gardenerControl->setGraftReceivers(0, sender);
        }
        send(gardenerControl, "gardenerControlOutput");
    }
}

void Cache::handleQuery(CacheQuery *msg)
{
    int contentId = msg->getContentId();
    bool found = contentIds.find(contentId) != contentIds.end();

    CacheQueryResponse *response = new CacheQueryResponse();
    response->setFound(found);

    cGate *arrivalGate = msg->getArrivalGate();
    int responseGateId = gateBaseId("queryPorts") + arrivalGate->getIndex();
    send(response, responseGateId);
}
