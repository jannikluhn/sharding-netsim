#ifndef UTILS_HUB_HUB_H_
#define UTILS_HUB_HUB_H_

#include <omnetpp.h>

using namespace omnetpp;


class Hub : public cSimpleModule {
  private:
    std::vector<int> node_ids;
    std::map<int, int> receiver_to_gate_ids;
    std::map<int, int> gate_to_sender_ids;
    simsignal_t message_sent_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_HUB_HUB_H_
