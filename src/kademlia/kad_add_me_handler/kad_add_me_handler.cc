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
    int shard = add_me->getShard();
    KadId kad_id = {sender, shard};

    if (peer_table->contains(kad_id)) {
        EV_WARN << "received add me from " << shard << "/" << sender << " who is already known"
            << endl;
    } else if (peer_table->insertPossible(kad_id)) {
        EV_DEBUG << "Inserting node " << shard << "/" << sender << " into peer table" << endl;
        peer_table->insert(kad_id);
    } else {
        // TODO: check if some node is offline
        EV_DEBUG << "Ignoring ADD_ME from " << shard << "/" << sender << " as bucket is full"
            << endl;
    }

    delete add_me;
}
