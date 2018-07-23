import random
random.seed(0)

import networkx as nx
import numpy as np

from toolz import (
    assoc,
    concat,
    merge,
)


OUTPUT_FILENAME = "network_adjlist"

config = {
    "NUM_NODES": 100000,
    "MIN_PEERS": 5,
}


def create_network(config): 
    num_nodes = config["NUM_NODES"]
    min_peers = config["MIN_PEERS"]
    assert num_nodes >= min_peers + 1

    g = nx.Graph()
    nodes = list(range(num_nodes))
    g.add_nodes_from(nodes)

    for node in nodes:
        while len(g[node]) < min_peers:
            potential_peer = random.choice(nodes)
            if potential_peer != node and potential_peer not in g[node]:
                g.add_edge(node, potential_peer)

    peer_numbers = [len(g[node]) for node in g.nodes]
    print(
        f"Successfully created network of {len(g.nodes)} nodes ({min(peer_numbers)} < peers < "
        f"{max(peer_numbers)}, median: {np.median(peer_numbers)}, avg: {np.average(peer_numbers)})"
    )

    return g


if __name__ == "__main__":
    g = create_network(config)
    nx.write_adjlist(g, OUTPUT_FILENAME)
