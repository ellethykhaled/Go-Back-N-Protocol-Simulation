#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include "InitMessage_m.h"
#include "FrameMessage_m.h"

#if !defined(FRAME_TYPE)
#define FRAME_TYPE
const int DATA = 0;
const int ACK = 1;
const int NACK = 2;
#endif


using namespace omnetpp;

class Node : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
  private:
    int nodeNumber;
    bool isSender;

    int sequenceNumber;

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
    void handleSender(int time, cMessage *msg);
    void handleReceiver(int time, cMessage *msg);
};

int lineCount; // Used to determine the last line read
void readLineFromFile(int nodeNumber, int &errorCode, std::string &message);
std::string getStuffedMessage(std::string message);
std::string calculateParityByte(std::string message);

#endif
