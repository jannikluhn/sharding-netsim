package sharding.kademlia.kad_ping_handler;

simple KadPingHandler
{
    parameters:
        string peerTablePath;
        volatile int shardId;

    gates:
        input pingInput;
        output out;
}
