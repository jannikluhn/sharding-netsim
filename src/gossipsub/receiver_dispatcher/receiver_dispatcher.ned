package sharding.gossipsub.receiver_dispatcher;


// Module that dispatches messages according to their receiver to the corresponding output port.
simple ReceiverDispatcher
{
    gates:
        input inputs[];
        output outputs[];
}
