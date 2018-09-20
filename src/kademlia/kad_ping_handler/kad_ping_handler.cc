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
    int sender = ping->getSender();
    int shard = ping->getShard();
    KadId kad_id = {sender, shard};
    peer_table->updateIfKnown(kad_id);

    KadPong *pong = new KadPong();
    pong->setShard(par("shardId").intValue());
    pong->setReceiver(sender);
    send(pong, "out");

    if (!par("hidden").boolValue()) {
        KadAddMe *add_me = new KadAddMe();
        add_me->setShard(par("shardId").intValue());
        add_me->setReceiver(sender);
        send(add_me, "out");
    }

    delete ping;
}
