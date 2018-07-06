import random
from typing import Generator
from statistics import median

import structlog
from simpy import Environment, Event

from network import Network


class ConnectionManager:

    logger = structlog.get_logger()

    def __init__(self, env: Environment, network: Network) -> None:
        self.env = env
        self.network = network

    def initialize_connections(self) -> None:
        raise NotImplementedError("Must be implemented by subclass")

    def run(self) -> Generator[Event, None, None]:
        raise NotImplementedError("Must be implemented by subclass")


class BasicConnectionManager(ConnectionManager):
    """Randomly connects peers until they have a good number."""

    def __init__(
        self, env: Environment, network: Network, min_peers: int, max_peers: int
    ) -> None:
        super().__init__(env, network)
        self.min_peers = min_peers
        self.max_peers = max_peers

    def initialize_connections(self) -> None:
        self.logger.info(
            "Initializing connections",
            num_nodes=len(self.network.nodes),
            min_peers=self.min_peers,
            max_peers=self.max_peers,
        )
        for node in random.sample(self.network.nodes, len(self.network.nodes)):
            required_peers = max(0, self.min_peers - node.num_peers)
            potential_peers = [
                peer
                for peer in self.network.nodes
                if peer not in node.peers and peer.num_peers < self.max_peers
            ]
            new_peers = random.sample(potential_peers, required_peers)

            for new_peer in new_peers:
                self.network.add_edge(node, new_peer)

            if node.num_peers < self.min_peers:
                self.logger.warning(
                    "Could not find enough peers",
                    node=node,
                    min_peers=self.min_peers,
                    num_peers=node.num_peers,
                )

        peer_numbers = [node.num_peers for node in self.network.nodes]
        self.logger.info(
            "Initialized connections",
            num_nodes=len(self.network.nodes),
            med_peers=median(peer_numbers),
            min_peers=min(peer_numbers),
            max_peers=max(peer_numbers),
        )

    def run(self) -> Generator[Event, None, None]:
        yield self.env.timeout(0)  # TODO
