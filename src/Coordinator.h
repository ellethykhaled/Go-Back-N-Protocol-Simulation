#ifndef __PROJECT_COORDINATOR_H_
#define __PROJECT_COORDINATOR_H_

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include "InitMessage_m.h"

#if !defined(MESSAGE_STATE)
#define MESSAGE_STATE
const std::string INIT = "Initialized";     // Indicates that the message is used for initialization
const std::string PROCESS = "Processing";   // Indicates that the message is used when processing
const std::string COMPLETE = "Complete";    // Indicates that the message is used when ready to be sent
#endif

using namespace omnetpp;

class Coordinator : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
  public:
    int WS;
    double TO;
    double PT;
    double TD;
    double ED;
    double DD;
    double LP;
  private:
    // A function that sets the parameters of the network
    void setParameters();
    void initializeNodes(int startTime, int startingNodeNumber);
};

int readCoordinatorFile(int &startTime);

#endif
