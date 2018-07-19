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

    for (int i = 0; i < peer_number; i++) {
        cGate *gate = node->gate("ports$o", i);
        cGate *receivingGate = gate->getNextGate();
        cModule *receivingModule = receivingGate->getOwnerModule();
        int receiverId = receivingModule->getId();

        eagerReceivers.insert(receiverId);
    }
}

void Gardener::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("graftInput")) {
        handleGraft(check_and_cast<Graft *>(msg));
    } else if (msg->arrivedOn("pruneInput")) {
        handlePrune(check_and_cast<Prune *>(msg));
    } else if (msg->arrivedOn("controlInput")) {
        handleControl(check_and_cast<GardenerControl *>(msg));
    } else if (msg->arrivedOn("eagerMulticastInputs")) {
        handleEagerMulticast(check_and_cast<AddressedPacket *>(msg));
    } else if (msg->arrivedOn("lazyMulticastInputs")) {
        handleLazyMulticast(check_and_cast<AddressedPacket *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }
    delete msg;
}

void Gardener::handleGraft(Graft *msg)
{
    // assume for the simulation that we know the message (otherwise an honest peer wouldn't have
    // sent GRAFT)
    Gossip *gossipMsg = new Gossip();
    int numContentIds = msg->getContentIdsArraySize();
    gossipMsg->setContentIdsArraySize(numContentIds);
    for (int i = 0; i < numContentIds; i++) {
        gossipMsg->setContentIds(i, msg->getContentIds(i));
    }
    send(gossipMsg, "out");

    int senderId = msg->getSender();
    eagerReceivers.insert(senderId);
    lazyReceivers.erase(senderId);
}

void Gardener::handlePrune(Prune *msg)
{
    int senderId = msg->getSender();
    lazyReceivers.insert(senderId);
    eagerReceivers.erase(senderId);
}

void Gardener::handleControl(GardenerControl *msg)
{
    for (int i = 0; i < msg->getGraftReceiversArraySize(); i++) {
        int receiverId = msg->getGraftReceivers(i);
        eagerReceivers.insert(receiverId);
        lazyReceivers.erase(receiverId);
    }
    for (int i = 0; i < msg->getPruneReceiversArraySize(); i++) {
        int receiverId = msg->getPruneReceivers(i);
        if (eagerReceivers.find(receiverId) != eagerReceivers.end()) {
            eagerReceivers.erase(receiverId);
            lazyReceivers.erase(receiverId);
            Prune *pruneMsg = new Prune();
            pruneMsg->setReceiver(receiverId);
            send(pruneMsg, "out");
        }
    }
}

void Gardener::handleEagerMulticast(AddressedPacket *msg)
{
    int relayerId = msg->getSender();
    for (auto receiverId : eagerReceivers) {
        if (receiverId != relayerId) {
            AddressedPacket *dupMsg = msg->dup();
            dupMsg->setReceiver(receiverId);
            send(dupMsg, "out");
        }
    }
}

void Gardener::handleLazyMulticast(AddressedPacket *msg)
{
    for (auto receiverId : lazyReceivers) {
        AddressedPacket *dupMsg = msg->dup();
        dupMsg->setReceiver(receiverId);
        send(dupMsg, "out");
    }
}
