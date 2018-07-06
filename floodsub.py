from typing import NamedTuple, Generator, List, DefaultDict
from collections import defaultdict

from simpy import Environment, Event

from network import Network, Node, Message, MessageID, NodeID


class MessageRecord(NamedTuple):
    message_id: MessageID
    timestamp: int


class FloodSubNode(Node):
    def __init__(
        self, env: Environment, network: Network, seen_message_ttl: int
    ) -> None:
        super().__init__(env, network)
        self.seen_messages: DefaultDict[NodeID, List[MessageRecord]] = defaultdict(list)
        self.seen_message_ttl = seen_message_ttl

    def has_seen(self, peer, message):
        seen_message_ids = [record.message_id for record in self.seen_messages[peer.id]]
        return message.id in seen_message_ids

    def set_seen(self, peer, message):
        if not self.has_seen(peer, message):
            record = MessageRecord(message.id, self.env.now)
            self.seen_messages[peer.id].append(record)

    def receive(self, sender: Node, message: Message) -> Generator[Event, None, None]:
        yield self.env.process(super().receive(sender, message))
        self.set_seen(sender, message)
        yield self.env.process(self.flood(message))

    def flood(self, message: Message) -> Generator[Event, None, None]:
        processes = [
            self.env.process(self.send(peer, message))
            for peer in self.peers
            if not self.has_seen(peer, message)
        ]
        yield self.env.all_of(processes)
        for peer in self.peers:
            self.set_seen(peer, message)

    def sweep_seen_messages(self):
        for node_id, message_records in self.seen_messages.items():
            sweeped_message_records = [
                record
                for record in message_records
                if record.timestamp < self.env.time - self.seen_message_ttl
            ]
            if not sweeped_message_records:
                self.seen_messages.pop(node_id)
            else:
                self.seen_messages[node_id] = sweeped_message_records

    def run(self) -> Generator[Event, None, None]:
        while True:
            yield self.env.timeout(self.seen_message_ttl)
            self.sweep_seen_messages()
