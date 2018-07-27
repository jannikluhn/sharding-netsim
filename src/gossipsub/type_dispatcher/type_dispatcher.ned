package sharding.gossipsub.type_dispatcher;


// Module that dispatches packets according to their type to corresponding outputs.
simple TypeDispatcher
{
    gates:
        input in;

        output getNodesOutputs[];
        output nodesOutputs[];
        output shuffleOutputs[];
        output shuffleReplyOutputs[];
        //output joinOutputs[];
        //output joinResponseOutputs[];

        output gossipOutputs[];
        output iHaveOutputs[];
        output graftOutputs[];
        output pruneOutputs[];
}
