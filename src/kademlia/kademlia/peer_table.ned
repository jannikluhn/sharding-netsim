package sharding.kademlia.kademlia;

simple KademliaPeerTable
{
    parameters:
        @signal[peerListUpdate](type=long);
}
