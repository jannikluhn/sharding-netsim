package sharding.utils.source;


// Module that creates messages in random intervals and sends them to its only output.
simple Source
{
    parameters:
        string cachePath;

        int nodeId;
        bool active = default(false);
        double rate @unit(Hz);
        double warmupTime @unit(s) = default(0);
        double lifeTime @unit(s);

        @signal[newGossipEmitted](long);

    gates:
        output outputs[];
}
