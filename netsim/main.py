import random
import time

import numpy as np
from toolz import count
import structlog

from netsim.network import Message
from netsim.connections import BasicConnectionManager
from netsim.floodsub import FloodSubNode, MessageRecord
from netsim.simulator import Simulator


logger = structlog.get_logger("floodsub")


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

        self.messages: Dict[MessageID, Message] = {}
        self.message_spawn_records: Dict[MessageID, MessageRecord] = {}

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

            message_record = MessageRecord(message.id, sender, self.env.now)
            self.messages[message.id] = message
            self.message_spawn_records[message.id] = message_record

            yield self.env.process(sender.flood(message))

    def prepare_processes(self):
        return super().prepare_processes() + [
            self.connection_manager.run(),
            self.run_tx_spawner(),
        ]

    def calc_reliability(self):
        if len(self.messages) == 0:
            raise ValueError("Cannot compute reliability as no message has been sent")

        n_nodes = len(self.network.nodes)
        summed_reliability = 0
        for message_id in self.messages:
            n_received = count(
                node for node in self.network.nodes
                if message_id in node.received_messages
            )
            summed_reliability += n_received / n_nodes

        return summed_reliability / len(self.messages)


if __name__ == "__main__":
    SIMULATION_TIME = 100

    sim = FloodSubSimulator()

    logger.info("Starting simulation", sim_time=SIMULATION_TIME)

    start_time = time.time()
    sim.run(SIMULATION_TIME)
    stop_time = time.time()

    logger.info("Simulation stopped", duration=(stop_time - start_time))

    print("Reliability: {}".format(sim.calc_reliability()))
