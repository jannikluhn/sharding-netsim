#include <omnetpp.h>
#include "../kademlia_packets_m.h"
#include "kad_find_node_handler.h"

using namespace omnetpp;


Define_Module(KadFindNodeHandler);


void KadFindNodeHandler::initialize() {
    const char *peer_table_path = par("peerTablePath").stringValue();
    peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));
}

void KadFindNodeHandler::handleMessage(cMessage *msg) {
    KadFindNode *find_node = check_and_cast<KadFindNode *>(msg);
    KadId sender_kad_id = find_node->getSenderKadId();
    KadId target_kad_id = find_node->getTargetKadId();

    peer_table->updateIfKnown(sender_kad_id);

    std::vector<KadId> neighbors = peer_table->getClosestPeers(target_kad_id, BUCKET_SIZE);
    EV_DEBUG << "received FindNode for " << target_kad_id << ", replying with "
        << neighbors.size() << " neighbors" << endl;

    KadNeighbors *reply = new KadNeighbors();
    reply->setSenderKadId(peer_table->getHomeId());
    reply->setKadIdsArraySize(neighbors.size());
    reply->setNodeIdsArraySize(neighbors.size());
    for (int i = 0; i < neighbors.size(); i++) {
        reply->setKadIds(i, neighbors[i]);
        reply->setNodeIds(i, peer_table->getNodeId(neighbors[i]));
    }
    reply->setReceiver(find_node->getSender());
    send(reply, "out");

    delete find_node;
}
