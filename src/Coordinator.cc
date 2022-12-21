#include "Coordinator.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    setParameters();
    int startTime;
    int startingNodeNumber = readCoordinatorFile(startTime);

    initializeNodes(startTime, startingNodeNumber);
}

void Coordinator::handleMessage(cMessage *msg)
{
}

void Coordinator::initializeNodes(int startTime, int startingNodeNumber)
{
    InitMessage * initMsgSender = new InitMessage("Initialization");
    initMsgSender->setWS(WS);
    initMsgSender->setTO(TO);
    initMsgSender->setPT(PT);
    initMsgSender->setTD(TD);
    initMsgSender->setED(ED);
    initMsgSender->setDD(DD);
    initMsgSender->setStartTime(startTime);
    initMsgSender->setStartingNode(startingNodeNumber);

    InitMessage * initMsgReceiver = new InitMessage("Initialization");
    initMsgReceiver->setPT(PT);
    initMsgReceiver->setTD(TD);
    initMsgReceiver->setLP(LP);
    initMsgReceiver->setStartingNode(startingNodeNumber);

    if (startingNodeNumber == 0){
        send(initMsgSender, "node0Out");
        send(initMsgReceiver, "node1Out");
    }
    else
    {
        send(initMsgSender, "node1Out");
        send(initMsgReceiver, "node0Out");
    }
}

void Coordinator::setParameters()
{
    // Reading the parameters from the network (in package.ned)
    WS = this->getParentModule()->par("WS");
    TO = this->getParentModule()->par("TO");
    PT = this->getParentModule()->par("PT");
    TD = this->getParentModule()->par("TD");
    ED = this->getParentModule()->par("ED");
    DD = this->getParentModule()->par("DD");
    LP = this->getParentModule()->par("LP");
}

int readCoordinatorFile(int &startTime)
{
    std::string fileName = "..\\inputFiles\\coordinator.txt";
    std::ifstream coordinatorFile(fileName);
    std::string firstLine;
    getline(coordinatorFile, firstLine);
    coordinatorFile.close();

    std::string number = "";
    bool isFirst = true;
    int startingNodeNumber;
    for (auto c : firstLine)
        if (c == ' ')
        {
            if (isFirst)
                startingNodeNumber = stoi(number);
            else
                startTime = stoi(number);
            isFirst = false;
            number = "";
        }
        else
            number = number + c;
    return startingNodeNumber;
}
