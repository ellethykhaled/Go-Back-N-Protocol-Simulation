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
    isProcessing = false;
    endOfMessages = false;
}

void Node::handleMessage(cMessage *msg)
{
    std::string type;
    if (isSender)
        type = "sender";
    else
        type = "receiver";
    EV << "Node" << nodeNumber << " handling as the " << type << endl;
    if (isSender)
        handleSender(msg);
    else
        handleReceiver(msg);
}

void Node::initializeNode(cMessage *msg)
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

            errorCodes = new int[WS];
            // Returns the actual start time in case of being the sender
            double startTime = receivedMessage->getStartTime();
            cMessage * awaking = new cMessage(FIRST.c_str());
            scheduleAt(simTime() + startTime, awaking);
        }
        else
        {
            isSender = false;
            WS = 1;
            PT = receivedMessage->getPT();
            TD = receivedMessage->getTD();
            LP = receivedMessage->getLP();
        }
    }
}

void Node::handleSender(cMessage *msg)
{
    // In case of initialization from co-ordintor
    if (strcmp(msg->getName(), INIT.c_str()) == 0)
    {
        initializeNode(msg);
        return;
    }

    // In case of self awaking message (scheduleAt) -OR- TIMEOUT
    if (strcmp(msg->getName(), PROCESS.c_str()) == 0)
    {
        FrameMessage* messageToSend = check_and_cast<FrameMessage *>(msg);
        applyEffectAndSend(messageToSend);
        isProcessing = false;
    }

    // In case of acknowledge or not acknowledge from the receiver
    if (strcmp(msg->getName(), COMPLETE.c_str()) == 0)
    {
        EV << "ACK received" << endl;
        return;
    }

    // Sets the node for the next event
    if (endOfMessages)
    {
        EV << "End of messages from sender" << endl;
        return;
    }

    FrameMessage * messageToSend = new FrameMessage(PROCESS.c_str());

    // Reading the next errorCode and message
    int errorCode;
    std::string message;

    readLineFromFile(nodeNumber, errorCode, message);
    // If there is nothing more to read
    if (message.empty())
    {
        endOfMessages = true;
        return;
    }

    message = getStuffedMessage(message);

    std::string parity = calculateParityByte(message);

    // Setting the Data
    messageToSend->setHeader(sequenceNumber);
    messageToSend->setPayload(message.c_str());
    messageToSend->setTrailer(parity.c_str());
    messageToSend->setFrameType(DATA);
    messageToSend->setAckNumber(sequenceNumber);
    errorCodes[sequenceNumber++] = errorCode;
    // Rotating sequenceNumber
    if (sequenceNumber == WS)
        sequenceNumber = 0;

    startProcessing(messageToSend);

    return;
}

void Node::handleReceiver(cMessage *msg)
{
    // In case the message is just for initialization, return
    if (strcmp(msg->getName(), INIT.c_str()) == 0)
    {
       initializeNode(msg);
       return;
    }

    // In case of self awaking message (scheduleAt) [finished Processing]
    if (strcmp(msg->getName(), PROCESS.c_str()) == 0)
    {
        isProcessing = false;
        FrameMessage *receivedMessage = check_and_cast<FrameMessage *>(msg);
        processReceivedMessage(receivedMessage);
        return;
    }


    if (strcmp(msg->getName(), COMPLETE.c_str()) == 0)
    {
       isProcessing = true;
       FrameMessage *receivedMessage = check_and_cast<FrameMessage *>(msg);
       if (receivedMessage != nullptr)
       {
           EV << "At receiver: " << receivedMessage->getPayload() << endl;
           receivedMessage->setName(PROCESS.c_str());
           scheduleAt(simTime() + PT, receivedMessage);
       }
       return;
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

    int i = 0;
    while (i++ <= lineCount)
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

void Node::startProcessing(FrameMessage* messageToSend)
{
    isProcessing = true;
    scheduleAt(simTime() + PT, messageToSend);
}

void Node::applyEffectAndSend(FrameMessage *message)
{
    // Process - Processed - Send
    EV << "Applying effect" << endl;
    int sNumber = message->getHeader();
    switch (errorCodes[sNumber])
    {
        case 0:         // No Error
        {
            EV << "Sender sending message with no errors" << endl;
            sendDelayed(message, simTime() + TD, "out");
            break;
        }
        case 1:         // Delay
        {
            EV << "Sender sending message with delay" << endl;
            sendDelayed(message, simTime() + TD + ED, "out");
            break;
        }
        case 2:         // Duplication
        {
            FrameMessage* message2 = message->dup();
            sendDelayed(message, simTime() + TD, "out");
            sendDelayed(message2, simTime() + TD + DD, "out");
            break;
        }
        case 3:         // Delay & Duplication
        {
            FrameMessage* message2 = message->dup();
            sendDelayed(message, simTime() + TD + ED, "out");
            sendDelayed(message2, simTime() + TD + ED + DD, "out");
            break;
        }
        case 4:         // Loss
    //        sendDelayed(message, simTime() + PT + DT, "out");
            break;
        default:
            break;
    }
    EV << "Message being sent" << endl;
}

void Node::processReceivedMessage(FrameMessage *message)
{

}
