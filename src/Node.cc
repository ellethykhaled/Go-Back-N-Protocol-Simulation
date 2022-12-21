#include "Node.h"
Define_Module(Node);

void Node::initialize()
{
    if(strcmp(getName(), "Node0"))
        nodeNumber = 0;
    else
        nodeNumber = 1;
}

void Node::handleMessage(cMessage *msg)
{
    int startTime = checkInit(msg);
}

int Node::checkInit(cMessage *msg)
{
    if (strcmp(msg->getName(), "Initialization") == 0)
    {
        InitMessage *receivedMessage = check_and_cast<InitMessage *>(msg);
        if (receivedMessage != nullptr)
        {
            if (receivedMessage->getStartingNode() == nodeNumber)
            {
                isSender = true;
                WS = receivedMessage->getWS();
                TO = receivedMessage->getTO();
                PT = receivedMessage->getPT();
                TD = receivedMessage->getTD();
                ED = receivedMessage->getED();
                DD = receivedMessage->getDD();
                // Returns the actual start time in case of being the sender
                return receivedMessage->getStartTime();
            }
            else
            {
                isSender = false;
                WS = 1;
                PT = receivedMessage->getPT();
                TD = receivedMessage->getTD();
                LP = receivedMessage->getLP();
                // Dummy return for the start time in case of being the receiver
                return 0;
            }
        }
    }
    return -1;
}
