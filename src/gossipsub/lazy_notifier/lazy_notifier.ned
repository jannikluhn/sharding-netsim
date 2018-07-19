package sharding.gossipsub.lazy_notifier;


// Module that regularly publishes IHave messages about new gossip.
simple LazyNotifier
{
    gates:
        input gossipInput;
        output notificationOutput;
}
