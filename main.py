import random

import numpy as np

from network import Message
from connections import BasicConnectionManager
from floodsub import FloodSubNode
from simulator import Simulator


class FloodSubSimulator(Simulator):

    config = {
        "NODE_NUMBER": 10,
        "MIN_PEERS": 5,
        "MAX_PEERS": 9,
        "SEEN_MESSAGE_TTL": 5 * 60,
        "TX_RATE": 10,
    }

    def __init__(self):
        super().__init__()
        self.connection_manager = BasicConnectionManager(
            self.env, self.network, self.config["MIN_PEERS"], self.config["MAX_PEERS"]
        )

    def setup_network(self):
        nodes = [
            FloodSubNode(self.env, self.network, self.config["SEEN_MESSAGE_TTL"])
            for _ in range(self.config["NODE_NUMBER"])
        ]
        self.network.add_nodes_from(nodes)
        self.connection_manager.initialize_connections()

        super().setup_network()

    def run_tx_spawner(self):
        mean_tx_interval = 1 / self.config["TX_RATE"]
        while True:
            yield self.env.timeout(np.random.exponential(mean_tx_interval))
            sender = random.choice(list(self.network.nodes))
            message = Message()
            yield self.env.process(sender.flood(message))

    def prepare_processes(self):
        return super().prepare_processes() + [
            self.connection_manager.run(),
            self.run_tx_spawner(),
        ]


if __name__ == "__main__":
    sim = FloodSubSimulator()
    sim.run(3)
