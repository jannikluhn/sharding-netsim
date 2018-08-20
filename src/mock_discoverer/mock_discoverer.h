#ifndef UTILS_MOCK_DISCOVERER_MOCK_DISCOVERER_H_
#define UTILS_MOCK_DISCOVERER_MOCK_DISCOVERER_H_

#include <omnetpp.h>

using namespace omnetpp;


class MockDiscoverer : public cSimpleModule {
  private:
    std::set<int> peers;

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const;
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_MOCK_DISCOVERER_MOCK_DISCOVERER_H_
