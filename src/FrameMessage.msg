packet FrameMessage {
    int header;
    string payload;
    string trailer;
    int frameType;
    int ackNumber;
}
