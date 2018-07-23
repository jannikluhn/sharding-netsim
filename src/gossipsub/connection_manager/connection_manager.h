#ifndef GOSSIPSUB_CONNECTION_MANAGER_CONNECTION_MANAGER_H_
#define GOSSIPSUB_CONNECTION_MANAGER_CONNECTION_MANAGER_H_

#include <omnetpp.h>

using namespace omnetpp;


class ConnectionManager : public cSimpleModule {
  private:
    int min_peers;
    int max_peers;

    std::set<int> connected_nodes;

    void handleJoin(Join *msg);
    void handleJoinResponse(JoinResponse *msg);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_CONNECTION_MANAGER_CONNECTION_MANAGER_H_
