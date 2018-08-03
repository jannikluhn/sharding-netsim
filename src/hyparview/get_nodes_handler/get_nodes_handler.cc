#include <omnetpp.h>
#include "get_nodes_handler.h"
#include "../../packets_m.h"

using namespace omnetpp;


Define_Module(GetNodesHandler);


void GetNodesHandler::initialize() {
    const char *peer_list_path = par("peerListPath").stringValue();
    peer_list = check_and_cast<PeerList *>(getModuleByPath(peer_list_path));
}

void GetNodesHandler::handleMessage(cMessage *msg) {
    GetNodes *get_nodes = check_and_cast<GetNodes *>(msg);

    Nodes *nodes = new Nodes();
    nodes->setReceiver(get_nodes->getSender());

    int num_peers = peer_list->getPeerListSize();
    nodes->setPeersArraySize(num_peers);
    for (int i = 0; i < num_peers; i++) {
        nodes->setPeers(i, peer_list->getPeerByIndex(i));
    }

    send(nodes, "out");

    delete msg;
}
