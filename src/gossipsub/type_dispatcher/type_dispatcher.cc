#include <omnetpp.h>
#include "../packets_m.h"
#include "type_dispatcher.h"

using namespace omnetpp;


Define_Module(TypeDispatcher);


void TypeDispatcher::handleMessage(cMessage *msg)
{
    std::string gateName;

    Gossip *gossip = dynamic_cast<Gossip *>(msg);
    if (gossip) {
        gateName = "gossipOutputs";
    }
    IHave *iHave = dynamic_cast<IHave *>(msg);
    if (iHave) {
        gateName = "iHaveOutputs";
    }
    Graft *graft = dynamic_cast<Graft *>(msg);
    if (graft) {
        gateName = "graftOutputs";
    }
    Prune *prune = dynamic_cast<Prune *>(msg);
    if (prune) {
        gateName = "pruneOutputs";
    }

    if (!gateName.empty()) {
        int baseId = gateBaseId("gossipOutputs");
        int size = gateSize("gossipOutputs");
        for (int i = 0; i < size; i++) {
            send(msg->dup(), baseId + i);
        }
    } else {
        EV_ERROR << "invalid packet type\n";
    }

    delete msg;
}
