#include <omnetpp.h>
#include "../packets_m.h"
#include "hub.h"

using namespace omnetpp;


Define_Module(Hub);


void Hub::initialize() {
    int in_base_id = gateBaseId("ports$i");
    int out_base_id = gateBaseId("ports$o");
    int gate_size = gateSize("ports");

    node_ids.reserve(gate_size);
    for (int i = 0; i < gate_size; i++) {
        cGate *out_gate = gate("ports$o", i);
        cGate *receiving_gate = out_gate->getNextGate();
        cModule *receiver = receiving_gate->getOwnerModule();
        int receiver_id = receiver->getIndex();

        node_ids.push_back(receiver_id);
        receiver_to_gate_ids[receiver_id] = out_base_id + i;
        gate_to_sender_ids[in_base_id + i] = receiver_id;
    }

    message_sent_signal = registerSignal("messageSent");
}

void Hub::handleMessage(cMessage *msg) {
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);

    int sender_id = gate_to_sender_ids[msg->getArrivalGateId()];
    addressed_packet->setSender(sender_id);

    int receiver_id = addressed_packet->getReceiver();
    if (receiver_id == -1) {
        // choose random node (for JOIN), but not sender
        do {
            receiver_id = node_ids[intuniform(0, node_ids.size() - 1)];
        } while (receiver_id == sender_id);
    }

    int gate_id = receiver_to_gate_ids[receiver_id];
    send(addressed_packet, gate_id);

    emit(message_sent_signal, addressed_packet->getId());
}
