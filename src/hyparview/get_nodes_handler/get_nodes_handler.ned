package sharding.hyparview.get_nodes_handler;

simple GetNodesHandler
{
    parameters:
        string peerListPath;
        int nodeId;

    gates:
        input getNodesInput;
        output out;
};
