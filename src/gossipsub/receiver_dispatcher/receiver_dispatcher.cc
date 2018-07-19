#include <omnetpp.h>
#include "../packets_m.h"
#include "receiver_dispatcher.h"

using namespace omnetpp;


Define_Module(ReceiverDispatcher);


void ReceiverDispatcher::initialize() {
    // note: requires the outputs to be directly connected to the ports of the node
    cModule *node = getParentModule();
    int base_id = gateBaseId("outputs");

    for (int i = 0; i < node->gateSize("ports"); i++) {
        cGate *gate = node->gate("ports$o", i);
        cGate *receiving_gate = gate->getNextGate();
        cModule *receiver = receiving_gate->getOwnerModule();

        receiver_to_gate_map[receiver->getId()] = base_id + i;
    }

    node_id = node->getId();
}

void ReceiverDispatcher::handleMessage(cMessage *msg) {
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);
    int receiver = addressed_packet->getReceiver();
    int gate_id = receiver_to_gate_map[receiver];

    addressed_packet->setSender(node_id);
    send(addressed_packet, gate_id);
}
