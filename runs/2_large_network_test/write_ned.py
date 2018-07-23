import random

import networkx as nx


NETWORK_FILENAME = "network_adjlist"
OUTPUT_NED_FILENAME = "network.ned"


NED_TEMPLATE = """package sharding.runs.run1_test_large_network;

import sharding.gossipsub.Node;


network Network
{{
    parameters:
        int nodeCount = {node_count};
        double gossipRate @unit(Hz);

        @statistic[newGossipEmitted](record=count);
        @statistic[newGossipReceived](record=count,mean,histogram);
        @statistic[messageSent](record=count);

    submodules:
        nodes[nodeCount]: Node {{
            source.rate = gossipRate / nodeCount;
        }}

    connections:
{connections}
}}"""

CONNECTION_TEMPLATE = "        nodes[{node1}].ports++ <--> {{ delay = {latency}s; }} <--> nodes[{node2}].ports++;"


def format_ned(graph, **kwargs):
    node_count = len(graph.nodes)
    connections = "\n".join(
        CONNECTION_TEMPLATE.format(
            node1=edge[0],
            node2=edge[1],
            latency=random.uniform(0.2, 1.5),
        )
        for edge in graph.edges
    )
    ned = NED_TEMPLATE.format(
        node_count=node_count,
        connections=connections,
        **kwargs,
    )
    return ned


if __name__ == "__main__":
    network = nx.read_adjlist(NETWORK_FILENAME)
    ned = format_ned(network)
    with open(OUTPUT_NED_FILENAME, "w") as f:
        f.write(ned)
