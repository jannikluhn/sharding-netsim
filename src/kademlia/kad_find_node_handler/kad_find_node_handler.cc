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
    int sender = find_node->getSender();
    int shard = find_node->getShard();
    KadId sender_kad_id = {sender, shard};
    int target_node = find_node->getTargetNode();
    int target_shard = find_node->getTargetShard();
    KadId target_kad_id = {target_node, target_shard};

    peer_table->updateIfKnown(sender_kad_id);

    std::vector<KadId> neighbors = peer_table->getNeighborsInBuckets(target_kad_id, BUCKET_SIZE);
    EV_DEBUG << "received FindNode for " << target_shard << "/" << target_node
        << ", replying with " << neighbors.size() << " neighbors" << endl;

    KadNeighbors *reply = new KadNeighbors();
    reply->setShard(par("shardId"));
    reply->setNodesArraySize(neighbors.size());
    reply->setShardsArraySize(neighbors.size());
    for (int i = 0; i < neighbors.size(); i++) {
        reply->setNodes(i, neighbors[i].node_id);
        reply->setShards(i, neighbors[i].shard_id);
    }
    reply->setReceiver(sender);
    send(reply, "out");

    delete find_node;
}
