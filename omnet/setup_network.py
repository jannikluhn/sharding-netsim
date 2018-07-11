import random
from statistics import median
import sys

import structlog
import networkx as nx
import numpy as np


structlog.configure(
    logger_factory=structlog.PrintLoggerFactory(sys.stderr),
)
logger = structlog.get_logger("setup")


NETWORK_TEMPLATE = """network FloodSub
{{
    parameters:
        int numNodes = {num_nodes};
        double txRate;

    submodules:
        node[numNodes]: FloodSubNode {{
            parameters:
                txRate = txRate;
        }}

    connections:
{connections}
}}"""


CONNECTION_TEMPLATE = "        node[{node1}].port++ <--> {{ delay = {latency}s; }} <--> node[{node2}].port++;"


config = {
    "NUM_NODES": 10,
    "MIN_PEERS": 2,
    "MAX_PEERS": 25,
    "LATENCY_MEAN": 0.1,
    "LATENCY_STD": 0.05,
}


def format_network(graph):
    num_nodes = len(graph.nodes)
    connections = "\n".join(
        CONNECTION_TEMPLATE.format(node1=edge[0], node2=edge[1], latency=g.edges[edge]["latency"])
        for edge in graph.edges
    )
    network = NETWORK_TEMPLATE.format(
        num_nodes=num_nodes,
        connections=connections,
    )
    return network


def create_network(config): 
    g = nx.Graph()
    g.add_nodes_from(range(config["NUM_NODES"]))

    for node in random.sample(g.nodes, len(g.nodes)):
        required_peers = max(0, config["MIN_PEERS"] - len(g[node]))
        potential_peers = [
            peer for peer in g.nodes
            if peer != node and peer not in g[node] and len(g[peer]) < config["MAX_PEERS"]
        ]
        new_peers = random.sample(potential_peers, required_peers)
        latencies = [
            abs(np.random.normal(config["LATENCY_MEAN"], config["LATENCY_STD"]))
            for _ in new_peers
        ]

        for new_peer, latency in zip(new_peers, latencies):
            g.add_edge(node, new_peer, latency=latency)

        if len(g[node]) < config["MIN_PEERS"]:
            logger.warning(
                "Could not find enough peers",
                node=node,
                min_peers=config["MIN_PEERS"],
                num_peers=len(g[node]),
            )

    peer_numbers = [len(g[node]) for node in g.nodes]
    logger.info(
        "Initialized connections",
        num_nodes=len(g.nodes),
        med_peers=median(peer_numbers),
        min_peers=min(peer_numbers),
        max_peers=max(peer_numbers),
    )

    return g


if __name__ == "__main__":
    g = create_network(config)
    print(format_network(g))
