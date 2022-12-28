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
            isSender = true;

            // Setting the sender node parameters from the omnetpp.ini
            WS = receivedMessage->getWS();
            TO = receivedMessage->getTO();
            PT = receivedMessage->getPT();
            TD = receivedMessage->getTD();
            ED = receivedMessage->getED();
            DD = receivedMessage->getDD();

            // Initializing the error and timeout arrays
            errorCodes = new int[WS];
            timeoutsAt = new simtime_t[WS];
            for (int i = 0; i < WS; i++)
                timeoutsAt[i] = INF;        // A great time instance

            // Initializing the message used in case of timeouts
            timeoutMessage = new cMessage(TIMEOUT.c_str());

            // Setting the last acknowledgment received by the greatest sequence number available
            lastAckReceived = WS - 1;

            // No error free lines
            errorFreeLine = -1;

            // Schedule a dummy message at the start time specified by the co-ordinator to start the actual simulation
            double startTime = receivedMessage->getStartTime();
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
    // Handle the initialization message sent by the co-ordinator
    if (strcmp(msg->getName(), INIT.c_str()) == 0)
    {
        initializeNode(msg);
        return;
    }

    // Handle the self awaking message received when the processing is done
    if (strcmp(msg->getName(), PROCESS_S.c_str()) == 0)
    {
        applyEffectAndSend();
        isProcessing = false;
        setTimeout();

        // This condition is used to return from the sender handler in case the window is full of messages
        if (sequenceNumber == lastAckReceived)
            return;
    }

    // Handle the self awaking message received when there is timeout in order to "Go Back N"
    if (strcmp(msg->getName(), TIMEOUT.c_str()) == 0)
    {
        // This variable holds the sequence number of the acknowledgment that should have been received
        int failedSequenceNumber = sequenceNumber + 1;
        if (failedSequenceNumber == WS)
            failedSequenceNumber = 0;

        EV << "Timeout event at time [" << simTime() << "], at Node[" << nodeNumber << "] for frame with seq_num = [" << failedSequenceNumber << "]\n";

        if (!endOfMessages)
            lineCount -= WS;                    // Go back N in case of not reaching the end of the file
        else
            lineCount = minimumLineCount - 1;   // In case of the end of the file, reach the minimumLineCount - 1 (as it does hold the lower bound + 1 in this case)

        // line count has a minimum value of 0
        if (lineCount < 0)
            lineCount = 0;

        errorFreeLine = lineCount;              // Set the error free line with the current line count that will be read

        // Resetting all timeouts
        for (int i = 0; i < WS; i++)
            timeoutsAt[i] = INF;
        endOfMessages = false;
    }

    // Handle the message sent by the receiver
    if (strcmp(msg->getName(), COMPLETE_R.c_str()) == 0)
    {
        // Cancel the scheduled timeout event
        cancelEvent(timeoutMessage);

        receivedMessage = check_and_cast<FrameMessage *>(msg);

        if (receivedMessage->getFrameType() == ACK)
        {
            for (int i = 0; i < WS; i++)
            {
                lastAckReceived++;
                if (lastAckReceived == WS)
                    lastAckReceived = 0;

                // Clear any timeout saved for any current sequence number and all previous sequence numbers
                // i.e. if the messages with sequence numbers 0, 1, 2, 3 are sent, and an Acknowledgment with sequence number 2 is received
                // Clear timeouts for 0, 1, 2 & leave the timeout for 3 as it is
                timeoutsAt[lastAckReceived] = INF;
                if (lastAckReceived == receivedMessage->getHeader())
                    break;
            }

            // Calculates the new lower bound (only correct not in the case of the end of the file)
            minimumLineCount = lineCount - WS;
            if (minimumLineCount < 0)
                minimumLineCount = 0;
            minimumLineCount += lastAckReceived + 1;

            setTimeout();
        }
        else
        {
            int receivedNackNumber = receivedMessage->getHeader();

            // "Going Back" till the line count reaches the line with message that caused not acknowledgment
            while (receivedNackNumber != lineCount % WS || lineCount == 0)
                lineCount--;

            // Force the line that caused an error to be error free
            errorFreeLine = lineCount;

//            EV << "At time [" << simTime() << "], at Node[" << nodeNumber << "] received NACK with seq_num = [" << lastAckReceived << "]\n";

            // Return and wait for its timeout
            return;
        }
//        EV << "At time [" << simTime() << "], at Node[" << nodeNumber << "] received ACK with seq_num = [" << lastAckReceived << "]\n";

        // If the acknowledgment is received during processing, wait for next self awaking message, else continue the handler
        if (isProcessing)
            return;
    }

    // Checking if the sender has reached the end of the input file
    if (endOfMessages)
    {
        // Checking if there are any timeouts set. A set timeout indicates that some acknowledgment is not received yet
        bool isFinale = true;
        for (int i = 0; i < WS; i++)
            if (timeoutsAt[i] < INF)
                isFinale = false;

        // If there are no scheduled timeouts found, terminate the connection
        if (isFinale)
            terminateConnection();

        return;
    }

    messageToSend = new FrameMessage(PROCESS_S.c_str());

    int errorCode;
    std::string message;

    // Reading the next errorCode and message
    readLineFromFile(nodeNumber, errorCode, message);

    // Enforce an error-free channel
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
    sequenceNumber = (lineCount - 1) % WS;          // Rotating sequence number
    messageToSend->setHeader(sequenceNumber);       // Sequence number
    messageToSend->setPayload(message.c_str());     // Stuffed message
    messageToSend->setTrailer(parity.c_str());      // Parity Byte
    messageToSend->setFrameType(DATA);              // Data message
    messageToSend->setAckNumber(sequenceNumber);    // Sequence number, again

    errorCodes[sequenceNumber] = errorCode;         // The error code

    // Set the timeout at the current time in addition to the processing time and timeout duration
    timeoutsAt[sequenceNumber] = simTime() + PT + TO;

    senderStartProcessing();
    isProcessing = true;
}

void Node::handleReceiver(cMessage *msg)
{
    // Handle the initialization message sent by the co-ordinator
    if (strcmp(msg->getName(), INIT.c_str()) == 0)
    {
       initializeNode(msg);
       return;
    }

    // Handle the self awaking message received when the processing is done
    if (strcmp(msg->getName(), PROCESS_R.c_str()) == 0)
    {
        receivedMessage = check_and_cast<FrameMessage *>(msg);
        sendReplyMessage();
        isProcessing = false;
        return;
    }

    // Handle the message sent by the sender
    if (strcmp(msg->getName(), COMPLETE_S.c_str()) == 0)
    {
        // Ignore the message in case the receiver is processing
       if (simTime() < whenFree)
          return;

       receivedMessage = check_and_cast<FrameMessage *>(msg);
       if (receivedMessage != nullptr)
       {
           // Ignore the message if it has a different sequence number
           if (receivedMessage->getHeader() != expectedSequenceNumber)
           {
//               EV << "At time " << simTime() <<" received refused the message, expected " << expectedSequenceNumber << " and got " << receivedMessage->getHeader() << endl;
               return;
           }

           isProcessing = true;
           receiverStartProcessing();
       }
    }
}

void Node::senderStartProcessing()
{
    EV << "At time [" << simTime() << "], Node [" << nodeNumber << "] introducing channel error with code = [" << errorCodes[messageToSend->getHeader()] << "]" << endl;
    isProcessing = true;

    // Schedule self awaking message after time PT
    scheduleAt(simTime() + PT, messageToSend);
}

void Node::applyEffectAndSend()
{
    // Duplicate message to avoid multiple use of the message
    FrameMessage * actualMessageToSend = messageToSend->dup();
    actualMessageToSend->setName(COMPLETE_S.c_str());

    // The next five variables are used just for the display message
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

    // Message sent in all cases
    EV << "At time [" << simTime() << "], Node [" << nodeNumber << "] [sent] frame with seq_num = [" << actualMessageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modifiedByteNumber << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";
    // Message sent in case of duplication
    if (duplicateNumber++ > 0)
        EV << "At time [" << simTime() + DD << "], Node [" << nodeNumber << "] [sent] frame with seq_num = [" << actualMessageToSend->getHeader() << "] and payload = [" << messageToSend->getPayload() << "] and trailer = [" << messageToSend->getTrailer() << "], Modified [" << modifiedByteNumber << "], Lost [" << isLost << "], Duplicate [" <<  duplicateNumber << "], Delay [" << delay << "]\n";

}

void Node::receiverStartProcessing()
{
    receivedMessage->setName(PROCESS_R.c_str());
    // Set the time instance at which the receiver is free again (after processing)
    whenFree = simTime() + PT;

    // SIM_MANIP here is used to enforce that the message being processed is processed
    // and a reply is sent (acknowledgment or not acknowledgment) before a new message is received from the sender
    scheduleAt(whenFree - SIM_MANIP, receivedMessage);
}

void Node::sendReplyMessage()
{
    // The next four variables define the type of the message and whether it is lost or not
    std::string isAck = "ACK";
    std::string parity = calculateParityByte(receivedMessage->getPayload());
    int ackNum = receivedMessage->getHeader();
    std::string isLost = "No";

    // In case the calculated parity does not match the received parity, set as Not acknowledgment
    if (strcmp(parity.c_str(), receivedMessage->getTrailer()) != 0)
        isAck = "NACK";
    // Generate an integer from 0 to 9
    int chance = rand() % 10;
    // If the number is less than the loss probability * 10 (say 1), the reply is lost
    if (chance < LP * 10)
        isLost = "Yes";
    EV << "At time [" << simTime() + SIM_MANIP << "], Node[" << nodeNumber << "] Sending [" << isAck << "] with number [" << ackNum << "], loss [" << isLost << "]\n";

    receivedMessage->setName(COMPLETE_R.c_str());
    receivedMessage->setHeader(ackNum);         // Sequence number
    receivedMessage->setPayload("");            // No payload (message)
    receivedMessage->setAckNumber(ackNum);      // Redundant sequence Number

    if (strcmp(isAck.c_str(), "ACK") == 0)
    {
        // Set the message type as acknowledgment
        receivedMessage->setFrameType(ACK);

        // Increment the expected sequence number
        expectedSequenceNumber++;
        if (expectedSequenceNumber == WS)
            expectedSequenceNumber = 0;
    }
    // Set the message type as not acknowledgment
    else
        receivedMessage->setFrameType(NACK);

    // In case it is not lost, send the message (considering the simulation manipulation)
    if (strcmp(isLost.c_str(), "No") == 0)
        sendDelayed(receivedMessage, TD + SIM_MANIP, "out");
}

void Node::setTimeout()
{
    // Cancel any scheduled timeout and set a new one which is the minimum within the saved timeouts
    int minIndex = 0;
    for (int i = 0; i < WS; i++)
        if (timeoutsAt[i] < timeoutsAt[minIndex])
            minIndex= i;
    if (timeoutMessage->isScheduled())
        cancelEvent(timeoutMessage);
    scheduleAt(timeoutsAt[minIndex], timeoutMessage);
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
    // If the character is '$' or '/' add '/' before it
    std::string result = "$";
    for (auto c : message){
        if (c == '$' || c == '/')
            result += "/";
        result += c;
    }
    return result + "$";

}

// Used for testing
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
