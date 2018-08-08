#ifndef UTILS_HUB_HUB_H_
#define UTILS_HUB_HUB_H_

#include <omnetpp.h>

using namespace omnetpp;


class Hub : public cSimpleModule {
  private:
    std::map<int, int> node_id_to_in_gates;
    std::map<int, int> node_id_to_out_gates;
    std::map<int, int> in_gate_to_node_ids;
    std::map<int, int> out_gate_to_node_ids;

    simsignal_t message_sent_signal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    void registerNode(int node_id, int in_gate_id, int out_gate_id);
    void deregisterNode(int node_id);
};


#endif  // UTILS_HUB_HUB_H_
