#include <omnetpp.h>
#include "../../packets_m.h"
#include "hub.h"

using namespace omnetpp;


Define_Module(Hub);


void Hub::initialize() {
    message_sent_signal = registerSignal("messageSent");
}

void Hub::handleMessage(cMessage *msg) {
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);

    int sender_id = in_gate_to_node_ids[msg->getArrivalGateId()];
    addressed_packet->setSender(sender_id);

    int receiver_id = addressed_packet->getReceiver();
    if (receiver_id == -1) {
        error("no receiver specified");
    }
    if (node_id_to_out_gates.count(receiver_id) == 0) {
        EV_FATAL << "received message from " << sender_id << " for " << receiver_id
            << " who does not exist" << endl;
        error("receiver does not exist");
    }

    int gate_id = node_id_to_out_gates[receiver_id];
    send(addressed_packet, gate_id);

    emit(message_sent_signal, addressed_packet->getId());
}

void Hub::registerNode(int node_id, int in_gate_id, int out_gate_id) {
    Enter_Method_Silent();

    if (node_id_to_out_gates.count(node_id) > 0) {
        error("node already registered");
    }
    if (in_gate_to_node_ids.count(in_gate_id)) {
        error("incoming gate already in use");
    }
    if (out_gate_to_node_ids.count(out_gate_id)) {
        error("outgoing gate already in use");
    }

    node_id_to_in_gates[node_id] = in_gate_id;
    node_id_to_out_gates[node_id] = out_gate_id;
    in_gate_to_node_ids[in_gate_id] = node_id;
    out_gate_to_node_ids[out_gate_id] = node_id;
}

void Hub::deregisterNode(int node_id) {
    Enter_Method_Silent();

    if (node_id_to_in_gates.count(node_id) == 0) {
        error("node not registered");
    }

    int in_gate_id = node_id_to_in_gates[node_id];
    int out_gate_id = node_id_to_out_gates[node_id];

    node_id_to_in_gates.erase(node_id);
    node_id_to_out_gates.erase(node_id);
    in_gate_to_node_ids.erase(in_gate_id);
    out_gate_to_node_ids.erase(out_gate_id);
}
