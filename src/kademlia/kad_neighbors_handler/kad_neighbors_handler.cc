#include <omnetpp.h>
#include "../kademlia_packets_m.h"
#include "kad_neighbors_handler.h"

using namespace omnetpp;


Define_Module(KadNeighborsHandler);


void KadNeighborsHandler::initialize() {
    const char *peer_table_path = par("peerTablePath").stringValue();
    peer_table = check_and_cast<KademliaPeerTable *>(getModuleByPath(peer_table_path));
}

void KadNeighborsHandler::handleMessage(cMessage *msg) {
    KadNeighbors *neighbors = check_and_cast<KadNeighbors *>(msg);

    int sender = neighbors->getSender();
    int shard = neighbors->getShard();
    KadId sender_kad_id = {sender, shard};
    peer_table->updateIfKnown(sender_kad_id);

    int known = 0;
    int unknown = 0;
    for (int i = 0; i < neighbors->getNodesArraySize(); i++) {
        int neighbor_node = neighbors->getNodes(i);
        int neighbor_shard = neighbors->getShards(i);
        KadId neighbor_kad_id = {neighbor_node, neighbor_shard};

        if (peer_table->contains(neighbor_kad_id)) {
            known += 1;
            peer_table->update(neighbor_kad_id);
        } else {
            unknown += 1;
            KadPing *ping = new KadPing();
            ping->setShard(par("shardId"));
            ping->setReceiver(neighbor_node);
            send(ping, "out");
        }
    }

    EV_DEBUG << "received NEIGHBORS from " << shard << "/" << sender << " with " << known
        << " known and " << unknown << " new nodes" << endl;

    delete neighbors;
}
