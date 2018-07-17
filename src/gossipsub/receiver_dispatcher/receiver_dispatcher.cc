#include <omnetpp.h>
#include "../packets_m.h"
#include "receiver_dispatcher.h"

using namespace omnetpp;


Define_Module(ReceiverDispatcher);


void ReceiverDispatcher::initialize()
{
    // note: requires the outputs to be directly connected to the ports of the node
    cModule *node = getParentModule();
    int baseId = gateBaseId("outputs");

    for (int i = 0; i < node->gateSize("ports"); i++) {
        cGate *gate = node->gate("ports$o", i);
        cGate *receivingGate = gate->getNextGate();
        cModule *receiver = receivingGate->getOwnerModule();

        receiverToGateMap[receiver->getId()] = baseId + i;
    }
}

void ReceiverDispatcher::handleMessage(cMessage *msg)
{
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);
    int receiver = addressed_packet->getReceiver();
    int gateId = receiverToGateMap[receiver];
    send(msg, gateId);
}
