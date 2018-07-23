#include <omnetpp.h>
#include "../packets_m.h"
#include "type_dispatcher.h"

using namespace omnetpp;


Define_Module(TypeDispatcher);


void TypeDispatcher::handleMessage(cMessage *msg) {
    int base_id;
    int size;
    bool recognized_message = false;

    Gossip *gossip = dynamic_cast<Gossip *>(msg);
    if (gossip) {
        base_id = gateBaseId("gossipOutputs");
        size = gateSize("gossipOutputs");
        recognized_message = true;
    }

    IHave *iHave = dynamic_cast<IHave *>(msg);
    if (iHave) {
        base_id = gateBaseId("iHaveOutputs");
        size = gateSize("iHaveOutputs");
        recognized_message = true;
    }

    Graft *graft = dynamic_cast<Graft *>(msg);
    if (graft) {
        base_id = gateBaseId("graftOutputs");
        size = gateSize("graftOutputs");
        recognized_message = true;
    }

    Prune *prune = dynamic_cast<Prune *>(msg);
    if (prune) {
        base_id = gateBaseId("pruneOutputs");
        size = gateSize("pruneOutputs");
        recognized_message = true;
    }

    Join *join = dynamic_cast<Join *>(msg);
    if (join) {
        base_id = gateBaseId("joinOutputs");
        size = gateSize("joinOutputs");
        recognized_message = true;
    }
    JoinResponse *join_response = dynamic_cast<JoinResponse *>(msg);
    if (join_response) {
        base_id = gateBaseId("joinResponseOutputs");
        size = gateSize("joinResponseOutputs");
        recognized_message = true;
    }

    if (recognized_message) {
        for (int i = 0; i < size; i++) {
            send(msg->dup(), base_id + i);
        }
    } else {
        EV_ERROR << "invalid packet type\n";
    }

    delete msg;
}
