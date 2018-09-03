#include <omnetpp.h>
#include "queue.h"

using namespace omnetpp;


Define_Module(Queue);


void Queue::initialize() {
    channel = gate("out")->getTransmissionChannel();
    send_next_packet_msg = new cMessage();
}

void Queue::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        // send next packet on queue
        cPacket *packet = queue.pop();
        send(packet, "out");

        // schedule next packet if any
        if (!queue.isEmpty()) {
            simtime_t finish_time = channel->getTransmissionFinishTime();
            scheduleAt(finish_time, send_next_packet_msg);
        }
    } else {
        cPacket *packet = check_and_cast<cPacket *>(msg);
        if (!channel->isBusy()) {
            // just send the packet if there's bandwidth
            send(packet, "out");
        } else {
            // otherwise schedule it
            queue.insert(packet);
            if (queue.getLength() == 1) {
                simtime_t finish_time = channel->getTransmissionFinishTime();
                scheduleAt(finish_time, send_next_packet_msg);
            }
        }
    }
}

Queue::~Queue() {
    cancelAndDelete(send_next_packet_msg);
}
