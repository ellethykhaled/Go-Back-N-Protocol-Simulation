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

#if !defined(MESSAGE_STATE)
#define MESSAGE_STATE
const std::string INIT = "Initialized";     // Indicates that the message is used for initialization
const std::string FIRST = "First";          // Indicates that the message is the first message to be sent
const std::string PROCESS = "Processing";   // Indicates that the message is used when processing
const std::string COMPLETE = "Complete";    // Indicates that the message is used when ready to be sent
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
    bool isProcessing;
    bool endOfMessages;

    int sequenceNumber;

    // Network parameters received initially from the coordinator
    int WS;
    double TO;
    double PT;
    double TD;
    double ED;
    double DD;
    double LP;

    int * errorCodes;

    // A function that sets the parameters when the message is an initialization message returning the start time
    // Otherwise returns -1
    void initializeNode(cMessage *msg);
    void handleSender(cMessage *msg);
    void handleReceiver(cMessage *msg);
    void applyEffectAndSend(FrameMessage *msg);
    void startProcessing(FrameMessage* messageToSend);
    void processReceivedMessage(FrameMessage *msg);
};

int lineCount; // Used to determine the last line read
void readLineFromFile(int nodeNumber, int &errorCode, std::string &message);
std::string getStuffedMessage(std::string message);
std::string calculateParityByte(std::string message);

#endif
