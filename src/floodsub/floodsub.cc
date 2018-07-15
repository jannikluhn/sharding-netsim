#include <set>
#include <omnetpp.h>

using namespace omnetpp;

#include "floodsub_m.h"


class SeenMsgFilter : public cSimpleModule
{
  private:
    std::set<long> seenMsgIds;
    simsignal_t msgSentSignal;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(SeenMsgFilter);


void SeenMsgFilter::initialize()
{
    msgSentSignal = registerSignal("msgSent");
}

void SeenMsgFilter::handleMessage(cMessage *msg)
{
    long msgId = msg->getTreeId();
    bool seen = seenMsgIds.find(msgId) != seenMsgIds.end();
    if (!seen) {
        seenMsgIds.insert(msgId);

        if (msg->arrivedOn("in")) {
            send(msg, "filteredIn");
        } else {
            send(msg, "filteredOut");
            emit(msgSentSignal, simTime());
        }
    } else {
        delete msg;
    }
}


class MsgSource : public cSimpleModule
{
  private:
    double rate;
    simsignal_t msgSpawnedSignal;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(MsgSource);


void MsgSource::initialize()
{
    rate = par("rate").doubleValue();

    msgSpawnedSignal = registerSignal("msgSpawned");

    cMessage *schedulerMsg = new cMessage();
    scheduleAt(simTime() + exponential(1. / rate), schedulerMsg);
}

void MsgSource::handleMessage(cMessage *schedulerMsg)
{
    FloodSubMsg *msg = new FloodSubMsg();
    msg->setHopCount(0);
    send(msg, "out");
    emit(msgSpawnedSignal, simTime());

    scheduleAt(simTime() + exponential(1. / rate), schedulerMsg);
}



class Flooder : public cSimpleModule
{
  private:
    simsignal_t msgReceivedSignal;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Flooder);


void Flooder::initialize()
{
    msgReceivedSignal = registerSignal("msgReceived");
}

void Flooder::handleMessage(cMessage *msg)
{
    FloodSubMsg *floodSubMsg = check_and_cast<FloodSubMsg *>(msg);

    emit(msgReceivedSignal, floodSubMsg->getHopCount());

    int outGateBaseId = gateBaseId("out");
    for (int i = 0; i < gateSize("out"); i++) {
        FloodSubMsg *dupMsg = floodSubMsg->dup();
        dupMsg->setHopCount(floodSubMsg->getHopCount() + 1);
        send(dupMsg, outGateBaseId + i);
    }
    delete msg;
}
