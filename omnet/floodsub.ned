network FloodSub
{
    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }

    submodules:
        node[2]: FloodSubNode {
            parameters:
                txRate = 1.0;
        }

    connections:
        node[0].port++ <--> Channel <--> node[1].port++;
}

// Create messages randomly with a certain average rate.
simple MsgSource
{
    parameters:
        double rate;

    gates:
        output out;
}

// Remember which messages came through and filter messages going through the out gate
// accordingly.
simple SeenMsgFilter
{
    gates:
        input in;
        output filteredIn;
        input out;
        output filteredOut;
}

// Send input coming from any incoming port to all outgoing ports
simple Flooder
{
    gates:
        input forwardInput[];
        output out[];
        input sourceInput;
}

module FloodSubNode
{
    parameters:
        double txRate;

    gates:
        inout port[];

    submodules:
        msgSource: MsgSource {
            rate = txRate / sizeof(port);
        }
        seenMsgFilter[sizeof(port)]: SeenMsgFilter;
        flooder: Flooder;

    connections:
        // connect ports with filters
        for i=0..sizeof(port) - 1 {
            port$i[i] --> seenMsgFilter[i].in;
            seenMsgFilter[i].filteredOut --> port$o[i];
        }

        // connect filters with flooder
        for i=0..sizeof(port) - 1 {
            seenMsgFilter[i].filteredIn --> flooder.forwardInput++;
            flooder.out++ --> seenMsgFilter[i].out;
        }

        // connect msg source to flooder
        msgSource.out --> flooder.sourceInput;
}
