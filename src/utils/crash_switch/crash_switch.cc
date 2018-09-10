#include <omnetpp.h>
#include "crash_switch.h"

using namespace omnetpp;


Define_Module(CrashSwitch);


void CrashSwitch::handleMessage(cMessage *msg) {
    if (par("crashed").boolValue()) {
        delete msg;
    } else {
        if (msg->arrivedOn("in")) {
            send(msg, "out");
        } else if (msg->arrivedOn("sourceInput")) {
            send(msg, "sourceOutput");
        } else {
            error("unhandled message");
        }
    }
}
