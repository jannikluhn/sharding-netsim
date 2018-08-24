#ifndef PUSHPULL_PULL_HANDLER_PULL_HANDLER_H_
#define PUSHPULL_PULL_HANDLER_PULL_HANDLER_H_

#include <omnetpp.h>
#include "../../utils/cache/cache.h"

using namespace omnetpp;


class PullHandler : public cSimpleModule {
  private:
    Cache *cache;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // PUSHPULL_PULL_HANDLER_PULL_HANDLER_H_
