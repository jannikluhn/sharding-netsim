from typing import Generator, List, Dict, Any

from simpy import Environment, Event

from network import Network


class Simulator:

    config: Dict[str, Any] = {}

    def __init__(self) -> None:
        self.env = Environment()
        self.network = Network(self.env)

    def setup_network(self) -> None:
        pass

    def prepare_processes(self) -> List[Generator[Event, None, None]]:
        return [node.run() for node in self.network.nodes]

    def run(self, time: int) -> None:
        self.setup_network()
        for g in self.prepare_processes():
            self.env.process(g)
        self.env.run(time)
