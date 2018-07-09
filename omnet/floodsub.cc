#include <set>
#include <omnetpp.h>
using namespace omnetpp;

class SeenMsgFilter : public cSimpleModule
{
  private:
    std::set<long> seenMsgIds;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(SeenMsgFilter);


void SeenMsgFilter::initialize()
{

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
        }
    } else {
        delete msg;
    }
}


class MsgSource : public cSimpleModule
{
  private:
    double rate;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(MsgSource);


void MsgSource::initialize()
{
    cMessage *schedulerMsg = new cMessage();
    scheduleAt(simTime() + exponential(1.0), schedulerMsg);
}

void MsgSource::handleMessage(cMessage *schedulerMsg)
{
    cMessage *msg = new cMessage();
    send(msg, "out");

    scheduleAt(simTime() + exponential(1.0), schedulerMsg);
}



class Flooder : public cSimpleModule
{
    protected:
      virtual void handleMessage(cMessage *msg);
};

Define_Module(Flooder);


void Flooder::handleMessage(cMessage *msg)
{
    int outGateBaseId = gateBaseId("out");
    for (int i = 0; i < gateSize("out"); i++) {
        cMessage *dupMsg = msg->dup();
        send(dupMsg, outGateBaseId + i);
    }
    delete msg;
}
