#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "connection_manager.h"

using namespace omnetpp;


Define_Module(ConnectionManager);


void ConnectionManager::initialize() {
    min_peers = par("minPeers").intValue();
    max_peers = par("maxPeers").intValue();

    Join *msg = new Join();
    send(msg, "out");
}

void ConnectionManager::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("joinInput")) {
        handleJoin(dynamic_cast<Join *>(msg));
    } else if (msg->arrivedOn("joinResponseInput")) {
        handleJoinResponse(dynamic_cast<JoinResponse *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }
    delete msg;
}

void ConnectionManager::handleJoin(Join *msg) {
    int peer_id = msg->getSender();

    JoinResponse *response = new JoinResponse();
    response->setReceiver(peer_id);

    if (connected_nodes.size() < max_peers) {
        response->setSuccess(true);
        connected_nodes.insert(peer_id);

        NewPeer *new_peer = new NewPeer();
        new_peer->setPeerId(peer_id);
        int new_peer_base_id = gateBaseId("newPeerOutputs");
        for (int i = 0; i < gateSize("newPeerOutputs"); i++) {
            send(new_peer->dup(), new_peer_base_id + i);
        }
        delete new_peer;
    } else {
        response->setSuccess(false);
    }

    send(response, "out");
}

void ConnectionManager::handleJoinResponse(JoinResponse *msg) {
    if (msg->getSuccess()) {
        connected_nodes.insert(msg->getSender());
    }

    if (connected_nodes.size() < min_peers) {
        Join *msg = new Join();
        send(msg, "out");
    }
}
