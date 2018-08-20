#include <omnetpp.h>
#include "../../packets_m.h"
#include "protocol_dispatcher.h"

using namespace omnetpp;


Define_Module(ProtocolDispatcher);


void ProtocolDispatcher::handleMessage(cMessage *msg) {
    AddressedPacket *addressed_packet = check_and_cast<AddressedPacket *>(msg);
    int protocol_class = addressed_packet->getProtocolClass();

    if (protocol_class == DISCOVERY_PROTOCOL) {
        send(msg, "discoveryOutput");
    } else if (protocol_class == GOSSIP_PROTOCOL) {
        send(msg, "gossipOutput");
    } else {
        error("unhandled message");
    }
}
