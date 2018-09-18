#include <omnetpp.h>
#include "../kademlia_packets_m.h"
#include "kad_add_me_handler.h"

using namespace omnetpp;


Define_Module(KadAddMeHandler);


void KadAddMeHandler::initialize() {
    const char *peer_table_path = par("peerTablePath").stringValue();
    peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));
}

void KadAddMeHandler::handleMessage(cMessage *msg) {
    KadAddMe *add_me = check_and_cast<KadAddMe *>(msg);
    int sender = add_me->getSender();
    int shard = add_me->getShardId();
    if (peer_table->contains(sender, shard)) {
        EV_ERROR << "received add me from " << shard << "/" << sender << " who is already known"
            << endl;
        error("received add me from known sender");
    }

    if (peer_table->insertPossible(sender, shard)) {
        EV_DEBUG << "Inserting node " << shard << "/" << sender << " into peer table" << endl;
        peer_table->insert(sender, shard);
    } else {
        EV_DEBUG << "Ignoring ADD_ME from " << shard << "/" << sender << " as bucket is full"
            << endl;
    }

    delete add_me;
}
