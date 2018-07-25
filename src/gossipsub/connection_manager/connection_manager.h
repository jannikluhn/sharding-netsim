#ifndef GOSSIPSUB_CONNECTION_MANAGER_CONNECTION_MANAGER_H_
#define GOSSIPSUB_CONNECTION_MANAGER_CONNECTION_MANAGER_H_

#include <omnetpp.h>

using namespace omnetpp;


enum class State {
    WAITING_FOR_NODES,
    JOINING,
};

class ConnectionManager : public cSimpleModule {
  private:
    State state;

    int c_rand;
    int min_peers;
    int join_ttl;
    int forward_join_ttl;

    std::vector<int> contact_nodes;
    std::vector<int> passive_list;
    std::vector<int> active_list;

    int node_id;

    void handleGetNodes(GetNodes *msg);
    void handleNodes(Nodes *msg);
    void handleJoin(Join *msg);
    void handleForwardJoin(ForwardJoin *msg);

    void addActivePeer(int node_id);
    void addPassivePeer(int node_id);
    void removeActivePeer(int node_id);
    void removePassivePeer(int node_id);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_CONNECTION_MANAGER_CONNECTION_MANAGER_H_
