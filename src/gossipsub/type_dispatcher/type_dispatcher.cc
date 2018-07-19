#include <omnetpp.h>
#include "../packets_m.h"
#include "type_dispatcher.h"

using namespace omnetpp;


Define_Module(TypeDispatcher);


void TypeDispatcher::handleMessage(cMessage *msg)
{
    int baseId;
    int size;
    bool recognizedMessage = false;

    Gossip *gossip = dynamic_cast<Gossip *>(msg);
    if (gossip) {
        baseId = gateBaseId("gossipOutputs");
        size = gateSize("gossipOutputs");
        recognizedMessage = true;
    }

    IHave *iHave = dynamic_cast<IHave *>(msg);
    if (iHave) {
        baseId = gateBaseId("iHaveOutputs");
        size = gateSize("iHaveOutputs");
        recognizedMessage = true;
    }

    Graft *graft = dynamic_cast<Graft *>(msg);
    if (graft) {
        baseId = gateBaseId("graftOutputs");
        size = gateSize("graftOutputs");
        recognizedMessage = true;
    }

    Prune *prune = dynamic_cast<Prune *>(msg);
    if (prune) {
        baseId = gateBaseId("pruneOutputs");
        size = gateSize("pruneOutputs");
        recognizedMessage = true;
    }

    if (recognizedMessage) {
        for (int i = 0; i < size; i++) {
            send(msg->dup(), baseId + i);
        }
    } else {
        EV_ERROR << "invalid packet type\n";
    }

    delete msg;
}
