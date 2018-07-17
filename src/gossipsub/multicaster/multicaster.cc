#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "multicaster.h"

using namespace omnetpp;


Define_Module(Multicaster);


void Multicaster::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("inputs")) {
        handleMulticast(dynamic_cast<AddressedPacket *>(msg));
    } else if (msg->arrivedOn("controlInput")) {
        handleControl(dynamic_cast<MulticastControl *>(msg));
    } else {
        EV_ERROR << "unhandled message\n";
    }
    delete msg;
}

void Multicaster::handleMulticast(AddressedPacket *msg)
{
    for (auto receiverId : receiverIds) {
        AddressedPacket *multicastMsg = msg->dup();
        multicastMsg->setReceiver(receiverId);
        send(multicastMsg, "out");
    }
}

void Multicaster::handleControl(MulticastControl *msg)
{
    for (int i = 0; i < msg->getAddReceiversArraySize(); i++) {
        receiverIds.insert(msg->getAddReceivers(i));
    }
    for (int i = 0; i < msg->getRemoveReceiversArraySize(); i++) {
        receiverIds.erase(msg->getRemoveReceivers(i));
    }
}
