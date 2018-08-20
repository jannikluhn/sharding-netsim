package sharding.utils.protocol_dispatcher;


simple ProtocolDispatcher
{
    parameters:
        @signal[messageSent](long);

    gates:
        input in;
        output gossipOutput;
        output discoveryOutput;
}
