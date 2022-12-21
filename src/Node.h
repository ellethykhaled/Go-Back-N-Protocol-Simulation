#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include "InitMessage_m.h"
#include "FrameMessage_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
  private:
    int nodeNumber;
    bool isSender;

    // Network parameters received initially from the coordinator
    int WS;
    double TO;
    double PT;
    double TD;
    double ED;
    double DD;
    double LP;

    // A function that sets the parameters when the message is an initialization message returning the start time
    // Otherwise returns -1
    int checkInit(cMessage *msg);
};

#endif
