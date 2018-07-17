package sharding.gossipsub.type_dispatcher;


// Module that dispatches packets according to their type to corresponding outputs.
simple TypeDispatcher
{
    gates:
        input inputs[];
        output gossipOutputs[];
        output iHaveOutputs[];
        output graftOutputs[];
        output pruneOutputs[];
}
