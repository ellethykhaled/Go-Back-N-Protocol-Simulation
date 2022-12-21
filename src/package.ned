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
        
    @display("bgb=322,240,white,black");
    submodules:
        Node1: Node {
            @display("p=269,22;i=device/switch");
        }
        Node0: Node {
            @display("p=52,25;i=device/switch");
        }
        coordinator: Coordinator {
            @display("p=168,156;i=block/cogwheel");
        }
}

