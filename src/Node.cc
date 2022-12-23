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
            // Setting the sender node parameters
            isSender = true;
            WS = receivedMessage->getWS();
            TO = receivedMessage->getTO();
            PT = receivedMessage->getPT();
            TD = receivedMessage->getTD();
            ED = receivedMessage->getED();
            DD = receivedMessage->getDD();

            errorCodes = new int[WS];
            lastAckReceived = WS - 1;
            errorFreeLine = -1;
            // Returns the actual start time in case of being the sender
            double startTime = receivedMessage->getStartTime();
            cMessage * awaking = new cMessage(FIRST.c_str());
            mQueue.push(msg);
            mQueue.push(awaking);
            scheduleAt(simTime() + startTime, awaking);
        }
        else
        {
            // Setting the receiver node parameters
            isSender = false;
            expectedSequenceNumber = 0;
            WS = receivedMessage->getWS();
            PT = receivedMessage->getPT();
            TD = receivedMessage->getTD();
            LP = receivedMessage->getLP();

            // A queue used to delete all messages when the simulation ends
            mQueue.push(msg);
        }
    }
}

void Node::handleSender(cMessage *msg)
{
    // Sender should stick to the window size and respond to NACKs

    // In case of initialization from co-ordintor
    if (strcmp(msg->getName(), INIT.c_str()) == 0)
    {
        initializeNode(msg);
        return;
    }

    // In case of self awaking message (scheduleAt)
    if (strcmp(msg->getName(), PROCESS_S.c_str()) == 0)
    {
        applyEffectAndSend();
        isProcessing = false;
        if (sequenceNumber == lastAckReceived)
        {
            setTimeout();
            return;
        }
    }

    // In case of TIMEOUT
    if (strcmp(msg->getName(), TIMEOUT.c_str()) == 0)
    {
        // Go back N
        int failedSequenceNumber = sequenceNumber + 1;
        if (failedSequenceNumber == WS)
            failedSequenceNumber = 0;

        EV << "Timeout event at time [" << simTime() << "], at Node[" << nodeNumber << "] for frame with seq_num = [" << failedSequenceNumber << "]\n";

        lineCount -= WS;
        if (lineCount < 0)
            lineCount = 0;
        errorFreeLine = lineCount;
    }

    // In case of acknowledge or not acknowledge from the receiver
    if (strcmp(msg->getName(), COMPLETE_R.c_str()) == 0)
    {
        cancelAndDelete(timeoutMessage);
        receivedMessage = check_and_cast<FrameMessage *>(msg);
        if (receivedMessage->getFrameType() == ACK)
            lastAckReceived = receivedMessage->getHeader();
        else
        {
            EV << "At time [" << simTime() << "], at Node[" << nodeNumber << "] received NACK with seq_num = [" << lastAckReceived << "]\n";
            return;
        }
        EV << "At time [" << simTime() << "], at Node[" << nodeNumber << "] received ACK with seq_num = [" << lastAckReceived << "]\n";
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
    if (errorFreeLine == lineCount - 1)
        errorCode = 0;
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

    startProcessing();
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
        receivedMessage = check_and_cast<FrameMessage *>(msg);
        sendReplyMessage();
        isProcessing = false;
        return;
    }


    if (strcmp(msg->getName(), COMPLETE_S.c_str()) == 0)
    {
       if (isProcessing)
           return;
       receivedMessage = check_and_cast<FrameMessage *>(msg);
       if (receivedMessage != nullptr)
       {
           if (receivedMessage->getHeader() != expectedSequenceNumber)
               return;
           isProcessing = true;
           processReceivedMessage();
       }
    }
}

void Node::startProcessing()
{
    EV << "At [" << simTime() << "], Node [" << nodeNumber << "] introducing channel error with code = [" << errorCodes[messageToSend->getHeader()] << "]" << endl;
    isProcessing = true;
    scheduleAt(simTime() + PT, messageToSend);
}

void Node::applyEffectAndSend()
{
    messageToSend->setName(COMPLETE_S.c_str());
    int sNumber = messageToSend->getHeader();
    std::string isLost = "No";
    int duplicateNumber = 0;
    int delay = 0;
    int modifiedByteNumber = -1;
    switch (errorCodes[sNumber])
    {
        case 0:         // No Error
        {
            sendDelayed(messageToSend, TD, "out");

            break;
        }
        case 1:         // Delay
        {
            delay = ED;

            sendDelayed(messageToSend, TD + ED, "out");

            break;
        }
        case 2:         // Duplication
        {
            duplicateMessageToSend = messageToSend->dup();
            duplicateNumber++;

            sendDelayed(messageToSend, TD, "out");
            sendDelayed(duplicateMessageToSend, TD + DD, "out");

            break;
        }
        case 3:         // Duplication & Delay
        {
            delay = ED;

            duplicateMessageToSend = messageToSend->dup();
            duplicateNumber++;

            sendDelayed(messageToSend, TD + ED, "out");
            sendDelayed(duplicateMessageToSend, TD + ED + DD, "out");

            break;
        }
        case 4:         // Loss
        {
            isLost = "Yes";
            break;
        }
        case 5:         // Loss & Delay
        {
            delay = ED;

            isLost = "Yes";
            break;
        }
        case 6:         // Loss & Duplication
        {
            duplicateNumber++;

            isLost = "Yes";
            break;
        }
        case 7:         // Loss, Duplication & Delay
        {
            delay = ED;

            duplicateNumber++;

            isLost = "Yes";
            break;
        }
        case 8:         // Modification
        {
            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            sendDelayed(messageToSend, TD, "out");

            break;
        }
        case 9:         // Modification & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            sendDelayed(messageToSend, TD + ED, "out");

            break;
        }
        case 10:         // Modification & Duplication
        {
            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            duplicateMessageToSend = messageToSend->dup();
            duplicateNumber++;

            sendDelayed(messageToSend, TD, "out");
            sendDelayed(duplicateMessageToSend, TD + DD, "out");

            break;
        }
        case 11:         // Modification, Duplication & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            duplicateMessageToSend = messageToSend->dup();
            duplicateNumber++;

            sendDelayed(messageToSend, TD + ED, "out");
            sendDelayed(duplicateMessageToSend, TD + ED + DD, "out");

            break;
        }
        case 12:         // Modification & Loss
        {
            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            isLost = "Yes";
            break;
        }
        case 13:         // Modification, Loss & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            isLost = "Yes";
            break;
        }
        case 14:         // Modification, Loss & Duplication
        {
            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            duplicateNumber++;

            isLost = "Yes";
            break;
        }
        case 15:         // Modification, Loss, Duplication & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(messageToSend->getPayload(), modifiedByteNumber);
            messageToSend->setPayload(modifiedPayload.c_str());

            duplicateNumber++;

            isLost = "Yes";
            break;
        }

        default:
            break;
    }
    EV << "At [" << simTime() << "], Node [" << nodeNumber << "] [sent] frame with seq_num = [" << messageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modifiedByteNumber << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";
    if (duplicateNumber++ > 0)
        EV << "At [" << simTime() + DD << "], Node [" << nodeNumber << "] [sent] frame with seq_num = [" << messageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modifiedByteNumber << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";

}

void Node::processReceivedMessage()
{
    receivedMessage->setName(PROCESS_R.c_str());
    scheduleAt(simTime() + PT, receivedMessage);
}

void Node::sendReplyMessage()
{
    // Receiver should only accept the expected frame

    std::string isAck = "ACK";
    std::string parity = calculateParityByte(receivedMessage->getPayload());
    int ackNum = receivedMessage->getHeader();
    if (ackNum == WS)
        ackNum = 0;
    if (strcmp(parity.c_str(), receivedMessage->getTrailer()) != 0)
        isAck = "NACK";
    std::string isLost = "No";
    int chance = rand() % 9;
    if (chance < LP * 10)
        isLost = "Yes";
    EV << "At time [" << simTime() << "], Node[" << nodeNumber << "] Sending [" << isAck << "] with number [" << ackNum << "], loss [" << isLost << "]\n";

    receivedMessage->setName(COMPLETE_R.c_str());
    receivedMessage->setHeader(ackNum);
    receivedMessage->setPayload("");
    if (strcmp(isAck.c_str(), "ACK") == 0)
    {
        receivedMessage->setFrameType(ACK);
        expectedSequenceNumber++;
        if (expectedSequenceNumber == WS)
            expectedSequenceNumber = 0;
    }
    else
        receivedMessage->setFrameType(NACK);

    if (strcmp(isLost.c_str(), "No") == 0)
        sendDelayed(receivedMessage, TD, "out");
}

void Node::setTimeout()
{
    timeoutMessage = new cMessage(TIMEOUT.c_str());
    scheduleAt(simTime() + TO, timeoutMessage);
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

std::string modifyPayload(std::string payload, int& byteNumber)
{
    std::string modifiedPayload = "";
    byteNumber = rand() % payload.size();

    int i = 0;
    for (auto c : payload)
    {
        if (i == byteNumber)
            // Modification of the (byteNumber)th byte to a random byte
            modifiedPayload += c + (rand() % 28);
        else
            modifiedPayload += c;
        i++;
    }
    return modifiedPayload;
}
