package project;

@license(LGPL);

network Network
{
    parameters:
        int WS;		// Sender window size
        double TO;	// Timeout interval
        double PT;	// Processing time for sender and receiver
        double TD;	// Channel's transmission delay
        double ED;	// Channel's error delay
        double DD;	// Channel's duplication delay
        double LP;	// Control frame loss probability

        @display("bgb=465,291,white,black");
    submodules:
        Node1: Node {
            @display("p=392,74;i=device/switch");
        }
        Node0: Node {
            @display("p=55,74;i=device/switch");
        }
        coordinator: Coordinator {
            @display("p=230,186;i=block/cogwheel");
        }
    connections:
        Node1.out --> { @display("ls=white"); } --> Node0.in;
        Node0.out --> { @display("ls=white"); } --> Node1.in;
        Node1.coordinatorOut --> coordinator.node1In;
        coordinator.node1Out --> Node1.coordinatorIn;
        Node0.coordinatorOut --> coordinator.node0In;
        coordinator.node0Out --> Node0.coordinatorIn;
}

