package sharding.gossipsub.receiver_dispatcher;


// Module that dispatches messages according to their receiver to the corresponding output port.
simple ReceiverDispatcher
{
    parameters:
        @signal[messageSent](long);

    gates:
        input inputs[];
        output outputs[];
}
