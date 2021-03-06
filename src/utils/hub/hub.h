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

    std::map<std::tuple<int, int, int>, simsignal_t> packet_sent_signals;
    std::map<int, simsignal_t> channel_used_signals;

  protected:
    virtual void handleMessage(cMessage *msg);

  public:
    void registerNode(int node_id, int in_gate_id, int out_gate_id);
    void deregisterNode(int node_id);
};


#endif  // UTILS_HUB_HUB_H_
