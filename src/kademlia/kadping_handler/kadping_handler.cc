#include <omnetpp.h>
#include "../kademlia_packets_m.h"
#include "kadping_handler.h"

using namespace omnetpp;


Define_Module(KadpingHandler);


void KadpingHandler::initialize() {
    const char *peer_table_path = par("peerTablePath").stringValue();
    peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));
}

void KadpingHandler::handleMessage(cMessage *msg) {
    KadPing *ping = check_and_cast<KadPing *>(msg);
    peer_table->updateIfKnown(ping->getSender(), ping->getShardId());

    KadPong *pong = new KadPong();
    pong->setShardId(par("shardId").intValue());
    pong->setReceiver(ping->getSender());
    send(pong, "out");
}
