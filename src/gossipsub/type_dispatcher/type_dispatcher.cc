#include <omnetpp.h>
#include "../packets_m.h"
#include "type_dispatcher.h"

using namespace omnetpp;


Define_Module(TypeDispatcher);


void TypeDispatcher::handleMessage(cMessage *msg) {
    GetNodes *getNodes = dynamic_cast<GetNodes *>(msg);
    if (getNodes) {
        return sendToOutputs(gateBaseId("getNodesOutputs"), gateSize("getNodesOutputs"), msg);
    }

    Nodes *nodes = dynamic_cast<Nodes *>(msg);
    if (nodes) {
        return sendToOutputs(gateBaseId("nodesOutputs"), gateSize("nodesOutputs"), msg);
    }

    Join *join = dynamic_cast<Join *>(msg);
    if (join) {
        return sendToOutputs(gateBaseId("joinOutputs"), gateSize("joinOutputs"), msg);
    }

    ForwardJoin *forward_join = dynamic_cast<ForwardJoin *>(msg);
    if (forward_join) {
        return sendToOutputs(gateBaseId("forwardJoinOutputs"), gateSize("forwardJoinOutputs"), msg);
    }

    Gossip *gossip = dynamic_cast<Gossip *>(msg);
    if (gossip) {
        return sendToOutputs(gateBaseId("gossipOutputs"), gateSize("gossipOutputs"), msg);
    }

    IHave *iHave = dynamic_cast<IHave *>(msg);
    if (iHave) {
        return sendToOutputs(gateBaseId("iHaveOutputs"), gateSize("iHaveOutputs"), msg);
    }

    Graft *graft = dynamic_cast<Graft *>(msg);
    if (graft) {
        return sendToOutputs(gateBaseId("graftOutputs"), gateSize("graftOutputs"), msg);
    }

    Prune *prune = dynamic_cast<Prune *>(msg);
    if (prune) {
        return sendToOutputs(gateBaseId("pruneOutputs"), gateSize("pruneOutputs"), msg);
    }

    // if we end up here something went wrong
    EV_ERROR << "invalid packet type\n";
    delete msg;
}

void TypeDispatcher::sendToOutputs(int base_id, int size, cMessage *msg) {
    for (int i = 0; i < size; i++) {
        send(msg->dup(), base_id + i);
    }
    delete msg;
}
