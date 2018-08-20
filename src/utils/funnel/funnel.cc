#include <omnetpp.h>
#include "funnel.h"

using namespace omnetpp;


Define_Module(Funnel);


void Funnel::handleMessage(cMessage *msg) {
    int base_gate_id = gateBaseId("outputs");
    for (int i = 0; i < gateSize("outputs"); i++) {
        send(msg->dup(), base_gate_id + i);
    }
    delete msg;
}
