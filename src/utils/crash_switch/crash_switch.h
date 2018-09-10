#ifndef UTILS_CRASH_SWITCH_CRASH_SWITCH_H_
#define UTILS_CRASH_SWITCH_CRASH_SWITCH_H_

#include <omnetpp.h>

using namespace omnetpp;


class CrashSwitch : public cSimpleModule {
  protected:
    virtual void handleMessage(cMessage *msg);
};


#endif  // UTILS_CRASH_SWITCH_CRASH_SWITCH_H_
