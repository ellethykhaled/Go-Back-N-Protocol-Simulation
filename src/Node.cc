#include "Node.h"
Define_Module(Node);

void Node::initialize()
{
    if(strcmp(getName(), "Node0") == 0)
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
            mQueue.push(msg);
            mQueue.push(awaking);
            scheduleAt(simTime() + startTime, awaking);
        }
        else
        {
            isSender = false;
            WS = receivedMessage->getWS();
            PT = receivedMessage->getPT();
            TD = receivedMessage->getTD();
            LP = receivedMessage->getLP();
            mQueue.push(msg);
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
    if (strcmp(msg->getName(), PROCESS_S.c_str()) == 0)
    {
        applyEffectAndSend();
        isProcessing = false;
    }

    // In case of acknowledge or not acknowledge from the receiver
    if (strcmp(msg->getName(), COMPLETE_R.c_str()) == 0)
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

    messageToSend = new FrameMessage(PROCESS_S.c_str());
    mQueue.push(messageToSend);

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
    isProcessing = true;
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
    if (strcmp(msg->getName(), PROCESS_R.c_str()) == 0)
    {
        isProcessing = false;

        receivedMessage = check_and_cast<FrameMessage *>(msg);
        processReceivedMessage();
        return;
    }


    if (strcmp(msg->getName(), COMPLETE_S.c_str()) == 0)
    {
        if (isProcessing)
            return;
       isProcessing = true;
       receivedMessage = check_and_cast<FrameMessage *>(msg);
       if (receivedMessage != nullptr)
       {
           receivedMessage->setName(PROCESS_R.c_str());
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
    EV << "At [" << simTime() << "], Node [" << nodeNumber << "] introducing channel error with code = [" << errorCodes[messageToSend->getHeader()] << "]" << endl;
    isProcessing = true;
    scheduleAt(simTime() + PT, messageToSend);
}

void Node::applyEffectAndSend()
{
    messageToSend->setName(COMPLETE_S.c_str());
    int sNumber = messageToSend->getHeader();
    int modified = -1;
    std::string isLost = "No";
    int duplicateNumber = 0;
    int delay = 0;
    switch (errorCodes[sNumber])
    {
        case 0:         // No Error
        {
            sendDelayed(messageToSend, simTime() + TD, "out");
            break;
        }
        case 1:         // Delay
        {
            delay = ED;
            sendDelayed(messageToSend, simTime() + TD + ED, "out");
            break;
        }
        case 2:         // Duplication
        {
            duplicateMessageToSend = messageToSend->dup();
            sendDelayed(messageToSend, simTime() + TD, "out");
            sendDelayed(duplicateMessageToSend, simTime() + TD + DD, "out");
            duplicateNumber++;
            break;
        }
        case 3:         // Delay & Duplication
        {
            delay = ED;
            duplicateMessageToSend = messageToSend->dup();
            sendDelayed(messageToSend, simTime() + TD + ED, "out");
            sendDelayed(duplicateMessageToSend, simTime() + TD + ED + DD, "out");
            duplicateNumber++;
            break;
        }
        case 4:         // Loss
    //        sendDelayed(message, simTime() + PT + DT, "out");
            break;
        default:
            break;
    }
    EV << "At [" << simTime() << "], Node [" << nodeNumber << "] [sent] frame with seq_num = [" << messageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modified << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";
}

void Node::processReceivedMessage()
{
    std::string isAck = "ACK";
    std::string parity = calculateParityByte(receivedMessage->getPayload());
    int ackNum = receivedMessage->getHeader() + 1;
    if (ackNum == WS)
        ackNum = 0;
    if (strcmp(parity.c_str(), receivedMessage->getTrailer()) != 0)
        isAck = "NACK";
    std::string isLost = "No";
    int chance = rand() % 9;
    if (chance < 1)
        isLost = "Yes";
    EV << "At time " << simTime() << ", Node[" << nodeNumber << "] Sending [" << isAck << "] with number [" << ackNum << "], loss [" << isLost << "]\n";
    if (strcmp(isLost.c_str(), "No") == 0)
    {
        receivedMessage->setName(COMPLETE_R.c_str());
        receivedMessage->setHeader(ackNum);
        if (strcmp(isAck.c_str(), "ACK") == 0)
            receivedMessage->setFrameType(ACK);
        else
            receivedMessage->setFrameType(NACK);

        sendDelayed(receivedMessage, simTime() + TD, "out");
    }
}

Node::~Node()
{
    if (nodeNumber == 1)
        return;
    int length = mQueue.size();
    for (int i = 0; i < length; i++)
    {
        cMessage* m = mQueue.front();
        mQueue.pop();
        delete m;
    }
}
