from __future__ import annotations

import itertools
from typing import Generator, NewType, Set

from toolz import assoc
import networkx as nx
from simpy import Environment, Event
import structlog


NodeID = NewType("NodeID", int)
MessageID = NewType("MessageID", int)


class Message:

    id_generator = itertools.count()

    def __init__(self):
        self.id = MessageID(next(self.id_generator))

    def __repr__(self) -> str:
        return "<Message {}>".format(self.id)


class Node:

    id_generator = itertools.count()

    def __init__(self, env: Environment, network: Network) -> None:
        self.env = env
        self.network = network

        self.id = NodeID(next(self.id_generator))

        self.logger = self.network.logger.bind(node=self)

    def __repr__(self) -> str:
        return "<Node {}>".format(self.id)

    def send(self, receiver: Node, message: Message) -> Generator[Event, None, None]:
        yield self.env.process(self.network.send(self, receiver, message))

    def receive(self, sender: Node, message: Message) -> Generator[Event, None, None]:
        self.logger.info("receiving message", msg=message)
        yield self.env.timeout(0)

    @property
    def peers(self) -> Set[Node]:
        return set(self.network.neighbors(self))

    @property
    def num_peers(self) -> int:
        return len(self.peers)

    def run(self) -> Generator[Event, None, None]:
        yield self.env.timeout(0)


class Network(nx.Graph):

    logger = structlog.get_logger()

    def __init__(self, env: Environment) -> None:
        self.env = env
        self.logger = structlog.wrap_logger(
            self.logger, processors=[lambda _, __, d: assoc(d, "time", self.env.now)]
        )
        super().__init__()

    def send(
        self, sender: Node, receiver: Node, message: Message
    ) -> Generator[Event, None, None]:
        if sender not in self:
            raise ValueError("Sender {} not in network".format(sender))
        if receiver not in self:
            raise ValueError("Receiver {} not in network".format(receiver))
        if receiver not in self.neighbors(sender):
            raise ValueError("{} and {} not connected".format(sender, receiver))

        delay = 0.1
        self.logger.info(
            "sending message",
            msg=message,
            sender=sender,
            receiver=receiver,
            delay=delay,
        )
        yield self.env.timeout(delay)
        yield self.env.process(receiver.receive(sender, message))
