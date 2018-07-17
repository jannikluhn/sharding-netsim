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

}

