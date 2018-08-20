package sharding.utils.type_dispatcher;


// Module that dispatches packets according to their type to corresponding outputs.
simple TypeDispatcher
{
    parameters:
        int numPacketTypes;

    gates:
        input in;

        output outputs[numPacketTypes];
}
