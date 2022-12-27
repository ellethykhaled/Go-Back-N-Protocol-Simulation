packet FrameMessage {
    int header;
    string payload;
    string trailer;
    int frameType;		// Data - ACK - NACK
    int ackNumber;
}
