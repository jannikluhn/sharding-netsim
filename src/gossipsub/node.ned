package sharding.gossipsub;

network TestNetwork
{
    submodules:
        nodes[3]: Node;

    connections:
        nodes[0].ports++ <--> nodes[1].ports++;
        nodes[0].ports++ <--> nodes[2].ports++;
        nodes[1].ports++ <--> nodes[2].ports++;
}

module Node
{
    gates:
        inout ports[];

    submodules:
        gossipSub: GossipSub;
        source: Source;

    connections:
        for i=0..sizeof(ports) - 1 {
            ports[i] <--> gossipSub.ports++;
        }
        source.out --> gossipSub.source;
}

simple GossipSub
{
    gates:
        inout ports[];
        input source;
}

simple Source
{
    gates:
        output out;
}
