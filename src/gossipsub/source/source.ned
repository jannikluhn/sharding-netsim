package sharding.gossipsub.source;


// Module that creates messages in random intervals and sends them to its only output.
simple Source
{
    parameters:
        string cachePath;

        int nodeId;
        bool active = default(false);
        double rate @unit(Hz);
        double startTime @unit(s) = default(0);
        double stopTime @unit(s) = default(-1);

        @signal[newGossipEmitted](long);

    gates:
        output outputs[];
}
