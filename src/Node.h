#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <queue>
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
const std::string INIT = "Initialized";                 // Indicates that the message is used for initialization
const std::string FIRST = "First";                      // Indicates that the message is the first message to be sent
const std::string PROCESS_S = "Sender Processing";      // Indicates a self awaking message indicating finished processing (sender)
const std::string COMPLETE_S = "Sender Complete";       // Used to mark data messages sent by sender
const std::string PROCESS_R = "Receiver Processing";    // Indicates a self awaking message indicating finished processing (receiver)
const std::string COMPLETE_R = "Receiver Complete";     // Used to mark control messages (ACK/NACK) sent by receiver
const std::string TIMEOUT = "Timeout";                  // Indicates that the sender has timed out
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

    // Used in case of sender
    FrameMessage * messageToSend;
    FrameMessage * duplicateMessageToSend;
    int sequenceNumber;
    bool endOfMessages;

    // Used in case of receiver
    FrameMessage * receivedMessage;
    int expectedSequenceNumber;


    // Network parameters received initially from the coordinator
    int WS;
    double TO;
    double PT;
    double TD;
    double ED;
    double DD;
    double LP;

    int * errorCodes;

    // A method that sets the parameters when the message is an initialization message returning the start time
    void initializeNode(cMessage *msg);

    // Main methods for sender and receiver
    void handleSender(cMessage *msg);
    void handleReceiver(cMessage *msg);

    // Methods used by sender for processing-sending
    void startProcessing();
    void applyEffectAndSend();

    // Methods used by receiver for processing-sending
    void processReceivedMessage();
    void sendReplyMessage();

    // Destructor used for clearing all messages in the global queue
    ~Node();
};

int lineCount;                  // Used to determine the last line read
std::queue<cMessage*> mQueue;   // A queue used to delete all messages when the simulation ends

// A function used to read
void readLineFromFile(int nodeNumber, int &errorCode, std::string &message);

// A function used to add stuffing and framing
std::string getStuffedMessage(std::string message);

// A function used to calculate the parity byte given the message string
std::string calculateParityByte(std::string message);

// A function that applies modification to the payload
std::string modifyPayload(std::string payload, int& bitNumber);

#endif
