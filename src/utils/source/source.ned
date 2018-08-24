package sharding.utils.source;


// Module that creates messages in random intervals and sends them to its only output.
simple Source
{
    parameters:
        double gossipRate @unit(Hz);
        bool periodic;
        double startTime @unit(s) = default(0);
        double stopTime @unit(s) = default(0);

        @signal[newGossipEmitted](long);

    gates:
        output outputs[];
}
