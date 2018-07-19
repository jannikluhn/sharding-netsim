#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "gardener.h"

using namespace omnetpp;


Define_Module(Gardener);


void Gardener::initialize()
{
    cModule *node = getParentModule();
    int peer_number = node->gateSize("ports");

    MulticastControl *multicast = new MulticastControl();
    multicast->setAddReceiversArraySize(peer_number);

    for (int i = 0; i < peer_number; i++) {
        cGate *gate = node->gate("ports$o", i);
        cGate *receivingGate = gate->getNextGate();
        cModule *receiver = receivingGate->getOwnerModule();
        int receiverId = receiver->getId();
        multicast->setAddReceivers(i, receiverId);
    }

    send(multicast, "eagerMulticastControlOutput");
}

void Gardener::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("graftInput")) {
        handleGraft(check_and_cast<Graft *>(msg));
    } else if (msg->arrivedOn("pruneInput")) {
        handlePrune(check_and_cast<Prune *>(msg));
    } else if (msg->arrivedOn("cacheQueryPort")) {
        handleCacheQueryResponse(check_and_cast<CacheQueryResponse *>(msg));
    } else if (msg->arrivedOn("controlInput")) {
        handleControl(check_and_cast<GardenerControl *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }
    delete msg;
}

void Gardener::handleGraft(Graft *msg)
{
    for (int i = 0; i < msg->getContentIdsArraySize(); i++) {
        // check if content is known (but only if the same request isn't underway already
        int contentId = msg->getContentIds(i);
        if (requests.count(contentId) == 0) {
            requests[contentId] = std::set<int>();

            CacheQuery *query = new CacheQuery();
            query->setContentId(contentId);
            send(query, "cacheQueryPort$o");
        }

        requests[contentId].insert(msg->getSender());
    }
}

void Gardener::handlePrune(Prune *msg)
{
    int receiverId = msg->getSender();
    prune(receiverId);
}

void Gardener::handleCacheQueryResponse(CacheQueryResponse *msg)
{
    int contentId = msg->getContentId();
    if (msg->getFound()) {
        for (auto receiverId : requests[contentId]) {
            Gossip *gossipMsg = new Gossip();
            gossipMsg->setContentIdsArraySize(1);
            gossipMsg->setContentIds(0, contentId);
            send(gossipMsg, "graftResponseOutput");
        }
    }
    requests.erase(contentId);
}

void Gardener::handleControl(GardenerControl *msg)
{
    for (int i = 0; i < msg->getPruneReceiversArraySize(); i++) {
        prune(msg->getPruneReceivers(i));
    }
    for (int i = 0; i < msg->getGraftReceiversArraySize(); i++) {
        graft(msg->getGraftReceivers(i));
    }
}

void Gardener::prune(int receiverId)
{
    MulticastControl *eagerControl = new MulticastControl();
    eagerControl->setRemoveReceiversArraySize(1);
    eagerControl->setRemoveReceivers(0, receiverId);
    send(eagerControl, "eagerMulticastControlOutput");

    MulticastControl *lazyControl = new MulticastControl();
    lazyControl->setAddReceiversArraySize(1);
    lazyControl->setAddReceivers(0, receiverId);
    send(lazyControl, "lazyMulticastControlOutput");
}

void Gardener::graft(int receiverId)
{
    MulticastControl *eagerControl = new MulticastControl();
    eagerControl->setAddReceiversArraySize(1);
    eagerControl->setAddReceivers(0, receiverId);
    send(eagerControl, "eagerMulticastControlOutput");

    MulticastControl *lazyControl = new MulticastControl();
    lazyControl->setRemoveReceiversArraySize(1);
    lazyControl->setRemoveReceivers(0, receiverId);
    send(lazyControl, "lazyMulticastControlOutput");
}
