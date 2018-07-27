#include <omnetpp.h>
#include "../../packets_m.h"
#include "../../internal_messages_m.h"
#include "passive_list_manager.h"

using namespace omnetpp;


Define_Module(PassiveListManager);


void PassiveListManager::initialize() {
    view_initialization_finished = false;
    node_id = par("nodeId").intValue();

    const char *peer_list_path = par("peerListPath").stringValue();
    peer_list = check_and_cast<PeerList *>(getModuleByPath(peer_list_path));

    startViewInitialization();
}

void PassiveListManager::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("nodesInput")) {
        handleNodes(check_and_cast<Nodes *>(msg));
    } else {
        error("unhandled message");
    }
    delete msg;
}


//
// View initialization
//
void PassiveListManager::startViewInitialization() {
    // send GETNODES to all passive peers (which at the moment should be just the contact nodes)
    for (int i = 0; i < peer_list->getPassiveListSize(); i++) {
        int receiver = peer_list->getPassivePeerByIndex(i);

        GetNodes *get_nodes = new GetNodes();
        get_nodes->setReceiver(receiver);
        send(get_nodes, "out");

        outstanding_getnodes_requests.insert(receiver);
    }
}

void PassiveListManager::handleNodes(Nodes *msg) {
    if (view_initialization_finished) {
        error("received unrequested NODES");
    }

    outstanding_getnodes_requests.erase(msg->getSender());

    for (int i = 0; i < msg->getPeersArraySize(); i++) {
        int peer_id = msg->getPeers(i);
        if (peer_id != node_id && !peer_list->isPeer(peer_id)) {
            peer_list->addPassivePeer(msg->getPeers(i));
        }
    }

    if (outstanding_getnodes_requests.empty()) {
        EV_INFO << "View initialization complete. Passive list size: "
            << peer_list->getPassiveListSize();

        view_initialization_finished = true;

        int gate_base_id = gateBaseId("viewInitializationFinishedOutputs");
        for (int i = 0; i < gateSize("viewInitializationFinishedOutputs"); i++) {
            send(new ViewInitializationComplete(), gate_base_id + i);
        }
    }
}
