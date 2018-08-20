#ifndef UTILS_SINK_SINK_H_
#define UTILS_SINK_SINK_H_

#include <omnetpp.h>

using namespace omnetpp;


class Sink : public cSimpleModule {
  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_SINK_SINK_H_
