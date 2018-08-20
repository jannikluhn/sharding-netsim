#include <omnetpp.h>
#include "sink.h"

using namespace omnetpp;


Define_Module(Sink);


void Sink::handleMessage(cMessage *msg) {
    delete msg;
}
