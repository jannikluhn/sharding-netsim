package sharding;


moduleinterface IGossiper
{
    parameters:
        int nodeId;

    gates:
        input in;
        input peerListChangeInput;
        output out;
}


moduleinterface IDiscoverer
{
    parameters:
        int nodeId;

    gates:
        input in;
        output peerListChangeOutput;
        output out;
}
