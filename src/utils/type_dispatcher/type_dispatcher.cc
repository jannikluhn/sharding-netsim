#include <omnetpp.h>
#include "../../packets_m.h"
#include "type_dispatcher.h"

using namespace omnetpp;


Define_Module(TypeDispatcher);


void TypeDispatcher::initialize() {
    num_packet_types = par("numPacketTypes").intValue();
}

void TypeDispatcher::handleMessage(cMessage *msg) {
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);
    int packet_type = addressed_packet->getPacketType();
    if (packet_type < 0) {
        error("Packet has no type");
    } else if (packet_type > num_packet_types) {
        error("Unknown packet type");
    } else {
        int output = gateBaseId("outputs") + packet_type;
        send(msg, output);
    }
}
