package sharding.kademlia.kadping_handler;

simple KadpingHandler
{
    parameters:
        string peerTablePath;
        volatile int shardId;

    gates:
        input pingInput;
        output out;
}
