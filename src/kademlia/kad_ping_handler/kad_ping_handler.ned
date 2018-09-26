package sharding.kademlia.kad_ping_handler;

simple KadPingHandler
{
    parameters:
        string peerTablePath;
        bool hidden;

    gates:
        input pingInput;
        output out;
}
