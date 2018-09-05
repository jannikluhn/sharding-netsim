package sharding;


moduleinterface IGossiper
{
    parameters:
        int nodeId;
        double datarate;

    gates:
        input in;
        input peerListChangeInput;
        input sourceInput;
        output out;
}


moduleinterface IDiscoverer
{
    parameters:
        int nodeId;
        double datarate;

    gates:
        input in;
        output peerListChangeOutput;
        output out;
}
