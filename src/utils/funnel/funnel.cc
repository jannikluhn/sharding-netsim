#include <omnetpp.h>
#include "funnel.h"

using namespace omnetpp;


Define_Module(Funnel);


void Funnel::handleMessage(cMessage *msg) {
    send(msg, "out");
}
