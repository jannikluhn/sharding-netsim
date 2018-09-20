package sharding.kademlia.kad_ping_handler;

simple KadPingHandler
{
    parameters:
        string peerTablePath;
        volatile int shardId;
        bool hidden;

    gates:
        input pingInput;
        output out;
}
