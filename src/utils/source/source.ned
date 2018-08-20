package sharding.utils.source;


// Module that creates messages in random intervals and sends them to its only output.
simple Source
{
    parameters:
        bool active = default(false);
        double gossipRate @unit(Hz);
        double startTime @unit(s) = default(0);
        double stopTime @unit(s) = default(0);

        @signal[newGossipEmitted](long);

    gates:
        output outputs[];
}
