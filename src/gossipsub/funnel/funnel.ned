package sharding.gossipsub.funnel;


// Module that funnels messages from many inputs to a single output.
simple Funnel
{
    gates:
        input inputs[];
        output out;
}
