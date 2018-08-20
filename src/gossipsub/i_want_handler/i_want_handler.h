#ifndef GOSSIPSUB_I_WANT_HANDLER_I_WANT_HANDLER_H_
#define GOSSIPSUB_I_WANT_HANDLER_I_WANT_HANDLER_H_

#include <omnetpp.h>
#include "../../utils/cache/cache.h"
#include "../gossipsub_packets_m.h"

using namespace omnetpp;


class IWantHandler : public cSimpleModule {
  private:
    Cache *cache;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif  // GOSSIPSUB_I_WANT_HANDLER_I_WANT_HANDLER_H_
