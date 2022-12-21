#include "Node.h"
Define_Module(Node);

void Node::initialize()
{
    if(strcmp(getName(), "Node0"))
        nodeNumber = 0;
    else
        nodeNumber = 1;
    sequenceNumber = 0;
    lineCount = 0;
}

void Node::handleMessage(cMessage *msg)
{
    int time = checkInit(msg);
    if (isSender)
        handleSender(time, msg);
    else
        handleReceiver(time, msg);
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

void Node::handleSender(int time, cMessage *msg)
{
    if (time != -1)
    {
        FrameMessage * messageToSend = new FrameMessage("");
        // Rotating sequenceNumber
        messageToSend->setHeader(sequenceNumber);

        // Reading the next errorCode and message
        int errorCode;
        std::string message;
        readLineFromFile(nodeNumber, errorCode, message);
        message = getStuffedMessage(message);
        messageToSend->setPayload(message.c_str());

        std::string parity = calculateParityByte(message);
        messageToSend->setTrailer(parity.c_str());

        messageToSend->setFrameType(DATA);

        messageToSend->setAckNumber(sequenceNumber++);
        if (sequenceNumber == WS)
            sequenceNumber = 0;

        sendDelayed(messageToSend, 5, "out");
        return;
    }
}

void Node::handleReceiver(int time, cMessage *msg)
{
    // In case the message is just for initialization, return
    if (time == 0)
        return;
    FrameMessage *receivedMessage = check_and_cast<FrameMessage *>(msg);
    if (receivedMessage != nullptr)
    {
        EV << receivedMessage->getPayload() << endl;
    }

}


void readLineFromFile(int nodeNumber, int &errorCode, std::string &message)
{
    std::string fileName;
    if (nodeNumber == 0)
        fileName = "..\\inputFiles\\input0.txt";
    else
        fileName = "..\\inputFiles\\input1.txt";

    std::ifstream senderFile(fileName);
    std::string firstLine;

    getline(senderFile, firstLine);
    lineCount++;
    senderFile.close();

    std::string word = "";
    bool isFirst = true;
    for (auto c : firstLine)
        if (c == ' ')
        {
            if (isFirst)
                errorCode = stoi(word);
            else
                message = word;
            isFirst = false;
            word = "";
        }
        else
            word = word + c;
}

std::string getStuffedMessage(std::string message)
{
    std::string result = "$";
    for (auto c : message){
        if (c == '$' || c == '/')
            result += "/";
        result += c;
    }
    return result + "$";

}

void printBinary(int n)
{
    int binaryNum[10];
    int i = 0;
    while (n > 0)
    {
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
    for (int j = i - 1; j >= 0; j--)
        EV << binaryNum[j];
    EV << "\n";
}

std::string calculateParityByte(std::string message)
{
    std::vector<std::bitset<8> > frameRepresentation;
    for (auto c : message)
        frameRepresentation.push_back(std::bitset<8>(c));

    frameRepresentation.push_back(frameRepresentation[0]);

    for (int i = 1; i < message.length(); i++)
        frameRepresentation[message.length()].operator ^=(frameRepresentation[i]);

//    int i = 0;
//    for (auto c : message)
//        EV << c << " " << frameRepresentation[i++] << endl;

    return frameRepresentation[message.length()].to_string();
}
