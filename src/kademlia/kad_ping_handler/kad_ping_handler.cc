#include <omnetpp.h>
#include "../kademlia_packets_m.h"
#include "kad_ping_handler.h"

using namespace omnetpp;


Define_Module(KadPingHandler);


void KadPingHandler::initialize() {
    const char *peer_table_path = par("peerTablePath").stringValue();
    peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));
}

void KadPingHandler::handleMessage(cMessage *msg) {
    KadPing *ping = check_and_cast<KadPing *>(msg);
    KadId kad_id = ping->getSenderKadId();
    int sender = ping->getSender();

    peer_table->updateIfKnown(kad_id);

    KadPong *pong = new KadPong();
    pong->setSenderKadId(peer_table->getHomeId());
    pong->setReceiver(sender);
    send(pong, "out");

    if (!par("hidden").boolValue()) {
        EV_DEBUG << "received PING from " << kad_id << ", replying with PONG and ADD_ME" << endl;

        KadAddMe *add_me = new KadAddMe();
        add_me->setSenderKadId(peer_table->getHomeId());
        add_me->setReceiver(sender);
        send(add_me, "out");
    } else {
        EV_DEBUG << "received PING from " << kad_id << ", replying with PONG" << endl;
    }

    delete ping;
}
