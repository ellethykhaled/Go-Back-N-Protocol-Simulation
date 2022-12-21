#ifndef __PROJECT_COORDINATOR_H_
#define __PROJECT_COORDINATOR_H_

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include "InitMessage_m.h"

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
