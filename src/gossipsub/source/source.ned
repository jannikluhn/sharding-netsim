package sharding.gossipsub.source;


// Module that creates messages in random intervals and sends them to its only output.
simple Source
{
    parameters:
        double rate @unit(Hz);

        @signal[newGossipEmitted](long);

    gates:
        output out;
}
