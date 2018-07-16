#include <omnetpp.h>

using namespace omnetpp;

#include "messages_m.h"


class Source : public cSimpleModule
{
  private:
    double rate;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Source);


void Source::initialize()
{
    //rate = par("rate").doubleValue();
    rate = 1.0;

    cMessage *schedulerMsg = new cMessage();
    scheduleAt(simTime() + exponential(1. / rate), schedulerMsg);
}

void Source::handleMessage(cMessage *schedulerMsg)
{
    Gossip *msg = new Gossip();
    msg->setContentId(msg->getTreeId());
    send(msg, "out");

    scheduleAt(simTime() + exponential(1. / rate), schedulerMsg);
}
