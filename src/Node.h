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

#if !defined(INFINITY_TIME)
#define INFINITY_TIME
const int INF = 10000;
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
    simtime_t whenFree;

    // Used in case of sender
    FrameMessage * messageToSend;           // Used to be sent as the main message
    FrameMessage * duplicateMessageToSend;  // Used to be sent as the duplicate message
    cMessage * timeoutMessage;              // Used as a self awaking message in case of timeout
    int sequenceNumber;
    bool endOfMessages;                     // A boolean indicating the end of messages in the node input file
    int lastAckReceived;                    // Stores the number of last Acknowledgment received
    int minimumLineCount;                   // Stores the lower bound when "Going Back N"

    // Used in case of receiver
    FrameMessage * receivedMessage;         // Used to hold the received message
    FrameMessage * messageToProcess;        // Used to gold the message being processed
    int expectedSequenceNumber;             // Holds the number of the next frame to accept

    simtime_t SIM_MANIP = 0.01;             // A number used to manipulate the simulation at receiver
                                            // The received gives undefined errors when processing two messages at the same time


    // Network parameters received initially from the coordinator
    int WS;         // Window Size
    double TO;      // Timeout
    double PT;      // Processing Time
    double TD;      // Transmission Delay
    double ED;      // Error Delay
    double DD;      // Duplication Delay
    double LP;      // Loss Probability

    int * errorCodes;           // An array if size "WS" used to store the errorCode mapped to the appropriate sequence number
    simtime_t * timeoutsAt;     // An array if size "WS" used to store the timeout for each message sent mapped to the appropriate sequence number
    int errorFreeLine;          // The integer storing the index of the line to send free of error in case of Timeout or NACK

    // A method that sets the parameters when the message is an initialization message returning the start time
    void initializeNode(cMessage *msg);

    // Main methods for sender and receiver
    void handleSender(cMessage *msg);
    void handleReceiver(cMessage *msg);

    // Methods used by sender for processing-sending
    void startProcessing();
    void applyEffectAndSend();
    void setTimeout();
    void terminateConnection();

    // Methods used by receiver for processing-sending
    void processReceivedMessage();
    void sendReplyMessage();
};

int lineCount;                  // Used to determine the last line read

// A function used to read
void readLineFromFile(int nodeNumber, int &errorCode, std::string &message);

// A function used to add stuffing and framing
std::string getStuffedMessage(std::string message);

// A function used to calculate the parity byte given the message string
std::string calculateParityByte(std::string message);

// A function that applies modification to the payload
std::string modifyPayload(std::string payload, int& bitNumber);

#endif
