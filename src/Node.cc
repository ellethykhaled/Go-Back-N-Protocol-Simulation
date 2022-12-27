#include "Node.h"
Define_Module(Node);

void Node::initialize()
{
    // Initializing the node number for each node
    if(strcmp(getName(), "Node0") == 0)
        nodeNumber = 0;
    else
        nodeNumber = 1;

    // Setting the essential variables used
    sequenceNumber = 0;     // Used by sender & receiver
    lineCount = 0;          // Used by sender
    isProcessing = false;   // Used by sender
    endOfMessages = false;  // Used by sender

}

void Node::handleMessage(cMessage *msg)
{
    // Send the message to the appropriate handler
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
            timeoutsAt = new simtime_t[WS];
            for (int i = 0; i < WS; i++)
                timeoutsAt[i] = INF;        // A great time instance
            lastAckReceived = WS - 1;
            errorFreeLine = -1;
            // Returns the actual start time in case of being the sender
            double startTime = receivedMessage->getStartTime();
            timeoutMessage = new cMessage(TIMEOUT.c_str());
            cMessage * awaking = new cMessage(FIRST.c_str());
            scheduleAt(simTime() + startTime, awaking);
        }
        else
        {
            // Setting the receiver node parameters
            isSender = false;
            expectedSequenceNumber = 0;
            whenFree = 0;
            WS = receivedMessage->getWS();
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

    // In case of self awaking message (scheduleAt)
    if (strcmp(msg->getName(), PROCESS_S.c_str()) == 0)
    {
        applyEffectAndSend();
        isProcessing = false;
        setTimeout();
        if (sequenceNumber == lastAckReceived)
            return;
    }

    // In case of TIMEOUT
    if (strcmp(msg->getName(), TIMEOUT.c_str()) == 0)
    {
        // Go back N
        int failedSequenceNumber = sequenceNumber + 1;
        if (failedSequenceNumber == WS)
            failedSequenceNumber = 0;

        EV << "Timeout event at time [" << simTime() << "], at Node[" << nodeNumber << "] with lineCount " << lineCount << " and minimumLineCount " << minimumLineCount << " for frame with seq_num = [" << failedSequenceNumber << "]\n";

        if (!endOfMessages)
            lineCount -= WS;        // Should get back to the correct line
        else
            lineCount = minimumLineCount - 1;
        if (lineCount < 0)
            lineCount = 0;
//        lineCount = minimumLineCount;
        errorFreeLine = lineCount;
        for (int i = 0; i < WS; i++)
            timeoutsAt[i] = INF;
        endOfMessages = false;
        EV << "Going back to line " << lineCount <<"\n";
    }

    // In case of acknowledge or not acknowledge from the receiver
    if (strcmp(msg->getName(), COMPLETE_R.c_str()) == 0)
    {
        cancelEvent(timeoutMessage);
//        EV << "At time "<< simTime() << " cancelled timeout\n";
        receivedMessage = check_and_cast<FrameMessage *>(msg);
        if (receivedMessage->getFrameType() == ACK)
        {
            for (int i = 0; i < WS; i++)
            {
                lastAckReceived++;
                if (lastAckReceived == WS)
                    lastAckReceived = 0;
                timeoutsAt[lastAckReceived] = INF;
                if (lastAckReceived == receivedMessage->getHeader())
                    break;
            }
            // MimimumLineCount should be adjusted
            minimumLineCount = lineCount - WS;
            if (minimumLineCount < 0)
                minimumLineCount = 0;
            minimumLineCount += lastAckReceived + 1;
//            lastAckReceived = receivedMessage->getHeader();
            setTimeout();
        }
        else
        {
            int receivedNackNumber = receivedMessage->getHeader();
            while (receivedNackNumber != lineCount % WS)
                lineCount--;
            errorFreeLine = lineCount;
            EV << "At time [" << simTime() << "], at Node[" << nodeNumber << "] received NACK with seq_num = [" << lastAckReceived << "]\n";
            return;
        }
        EV << "At time [" << simTime() << "], at Node[" << nodeNumber << "] received ACK with seq_num = [" << lastAckReceived << "]\n";
        delete receivedMessage;
        if (isProcessing)
            return;
    }

    // Sets the node for the next event
    if (endOfMessages)
    {
        bool isFinale = true;
        for (int i = 0; i < WS; i++)
            if (timeoutsAt[i] < INF)
                isFinale = false;

        if (isFinale)
        {
            terminateConnection();
        }

        return;
    }

    messageToSend = new FrameMessage(PROCESS_S.c_str());

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
    sequenceNumber = (lineCount - 1) % WS;       // Rotating sequence number
    messageToSend->setHeader(sequenceNumber);
    messageToSend->setPayload(message.c_str());
    messageToSend->setTrailer(parity.c_str());
    messageToSend->setFrameType(DATA);
    messageToSend->setAckNumber(sequenceNumber);
    errorCodes[sequenceNumber] = errorCode;
    timeoutsAt[sequenceNumber] = simTime() + PT + TO;

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
       if (simTime() < whenFree)
          return;

       receivedMessage = check_and_cast<FrameMessage *>(msg);
       if (receivedMessage != nullptr)
       {
//           int nextExpected = expectedSequenceNumber + 1;
//           if (nextExpected == WS)
//               nextExpected = 0;
//           bool iAmTheChosenOne = receivedMessage->getHeader() == nextExpected && whenFree == simTime();
           if (receivedMessage->getHeader() != expectedSequenceNumber)
           {
//               EV << "At time " << simTime() <<" received refused the message, expected " << expectedSequenceNumber << " and got " << receivedMessage->getHeader() << endl;
               return;
           }

           isProcessing = true;
           processReceivedMessage();
       }
    }
}

void Node::startProcessing()
{
    EV << "At time [" << simTime() << "], Node [" << nodeNumber << "] introducing channel error with code = [" << errorCodes[messageToSend->getHeader()] << "]" << endl;
    isProcessing = true;
    scheduleAt(simTime() + PT, messageToSend);
}

void Node::applyEffectAndSend()
{
    FrameMessage * actualMessageToSend = messageToSend->dup();
    actualMessageToSend->setName(COMPLETE_S.c_str());
    int sNumber = actualMessageToSend->getHeader();
    std::string isLost = "No";
    int duplicateNumber = 0;
    int delay = 0;
    int modifiedByteNumber = -1;
    switch (errorCodes[sNumber])
    {
        case 0:         // No Error
        {
            sendDelayed(actualMessageToSend, TD, "out");
            break;
        }
        case 1:         // Delay
        {
            delay = ED;

            sendDelayed(actualMessageToSend, TD + ED, "out");

            break;
        }
        case 10:         // Duplication
        {
            duplicateMessageToSend = actualMessageToSend->dup();
            duplicateNumber++;

            sendDelayed(actualMessageToSend, TD, "out");
            sendDelayed(duplicateMessageToSend, TD + DD, "out");

            break;
        }
        case 11:         // Duplication & Delay
        {
            delay = ED;

            duplicateMessageToSend = actualMessageToSend->dup();
            duplicateNumber++;

            sendDelayed(actualMessageToSend, TD + ED, "out");
            sendDelayed(duplicateMessageToSend, TD + ED + DD, "out");

            break;
        }
        case 100:         // Loss
        {
            isLost = "Yes";
            break;
        }
        case 101:         // Loss & Delay
        {
            delay = ED;

            isLost = "Yes";
            break;
        }
        case 110:         // Loss & Duplication
        {
            duplicateNumber++;

            isLost = "Yes";
            break;
        }
        case 111:         // Loss, Duplication & Delay
        {
            delay = ED;

            duplicateNumber++;

            isLost = "Yes";
            break;
        }
        case 1000:         // Modification
        {
            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            sendDelayed(actualMessageToSend, TD, "out");

            break;
        }
        case 1001:         // Modification & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            sendDelayed(actualMessageToSend, TD + ED, "out");

            break;
        }
        case 1010:         // Modification & Duplication
        {
            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            duplicateMessageToSend = actualMessageToSend->dup();
            duplicateNumber++;

            sendDelayed(actualMessageToSend, TD, "out");
            sendDelayed(duplicateMessageToSend, TD + DD, "out");

            break;
        }
        case 1011:         // Modification, Duplication & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            duplicateMessageToSend = actualMessageToSend->dup();
            duplicateNumber++;

            sendDelayed(actualMessageToSend, TD + ED, "out");
            sendDelayed(duplicateMessageToSend, TD + ED + DD, "out");

            break;
        }
        case 1100:         // Modification & Loss
        {
            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            isLost = "Yes";
            break;
        }
        case 1101:         // Modification, Loss & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            isLost = "Yes";
            break;
        }
        case 1110:         // Modification, Loss & Duplication
        {
            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            duplicateNumber++;

            isLost = "Yes";
            break;
        }
        case 1111:         // Modification, Loss, Duplication & Delay
        {
            delay = ED;

            std::string modifiedPayload = modifyPayload(actualMessageToSend->getPayload(), modifiedByteNumber);
            actualMessageToSend->setPayload(modifiedPayload.c_str());

            duplicateNumber++;

            isLost = "Yes";
            break;
        }

        default:
            break;
    }
    EV << "At time [" << simTime() << "], Node [" << nodeNumber << "] read line [" << lineCount - 1 << "] [sent] frame with seq_num = [" << actualMessageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modifiedByteNumber << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";
    if (duplicateNumber++ > 0)
        EV << "At time [" << simTime() + DD << "], Node [" << nodeNumber << "] [sent] frame with seq_num = [" << actualMessageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modifiedByteNumber << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";

}

void Node::processReceivedMessage()
{
    receivedMessage->setName(PROCESS_R.c_str());
    whenFree = simTime() + PT;
    scheduleAt(whenFree - SIM_MANIP, receivedMessage);
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
    EV << "At time [" << simTime() + SIM_MANIP << "], Node[" << nodeNumber << "] Sending [" << isAck << "] with number [" << ackNum << "], loss [" << isLost << "]\n";

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
        sendDelayed(receivedMessage, TD + SIM_MANIP, "out");
}

void Node::setTimeout()
{ // Timeout should be set according to the oldest frame sent
    // Cancel any scheduled timeout and set a new one
    int min = 0;
    for (int i = 0; i < WS; i++)
        if (timeoutsAt[i] < timeoutsAt[min])
            min = i;
    if (timeoutMessage->isScheduled())
        cancelEvent(timeoutMessage);
    scheduleAt(timeoutsAt[min], timeoutMessage);
//    scheduleAt(simTime() + TO, timeoutMessage);
}

void Node::terminateConnection()
{
    EV << "End of messages from sender, terminating connection..." << endl;
    // Canceling the dummy timeout set at infinity
    cancelEvent(timeoutMessage);
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
