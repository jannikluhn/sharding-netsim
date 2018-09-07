#include <omnetpp.h>
#include "../../packets_m.h"
#include "hub.h"

using namespace omnetpp;


Define_Module(Hub);


void Hub::handleMessage(cMessage *msg) {
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);

    int sender_id = in_gate_to_node_ids[msg->getArrivalGateId()];
    addressed_packet->setSender(sender_id);

    emit(channel_used_signals[sender_id], addressed_packet->getDuration());


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

    int packet_type = addressed_packet->getPacketType();
    int protocol = addressed_packet->getProtocol();
    int protocol_class = addressed_packet->getProtocolClass();
    std::tuple<int, int, int> packet_tuple = std::make_tuple(protocol_class, protocol, packet_type);
    if (packet_sent_signals.count(packet_tuple) == 0) {
        std::string packet_id_suffix = "-" + std::to_string(protocol_class) + "-" +
            std::to_string(protocol) + "-" + std::to_string(packet_type);

        std::string signal_name = "packetSent" + packet_id_suffix;
        simsignal_t signal = registerSignal(signal_name.c_str());
        packet_sent_signals[packet_tuple] = signal;

        std::string statistic_name = "packetSent" + packet_id_suffix;
        cProperty *statistic_template = getProperties()->get(
            "statisticTemplate",
            "packetSent"
        );
        getEnvir()->addResultRecorders(this, signal, statistic_name.c_str(), statistic_template);
    }
    emit(packet_sent_signals[packet_tuple], true);
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


    std::string signal_name = "channelUsed-" + std::to_string(node_id);
    simsignal_t signal = registerSignal(signal_name.c_str());
    channel_used_signals[node_id] = signal;

    std::string statistic_name = "channelUsed-" + std::to_string(node_id);
    cProperty *statistic_template = getProperties()->get(
        "statisticTemplate",
        "channelUsed"
    );
    getEnvir()->addResultRecorders(this, signal, statistic_name.c_str(), statistic_template);
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
