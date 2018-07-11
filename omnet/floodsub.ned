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
            seenMsgFilter[i].filteredIn --> flooder.in++;
            flooder.out++ --> seenMsgFilter[i].out;
        }

        // connect msg source to flooder
        msgSource.out --> flooder.in++;
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
    parameters:
        @signal[msgReceived](type="long");
        @statistic[hopCount](title="hop count"; source="msgReceived"; record=vector,stats; interpolationmode=none);
    gates:
        input in[];
        output out[];
}
