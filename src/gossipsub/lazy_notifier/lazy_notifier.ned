package sharding.gossipsub.lazy_notifier;


// Module that regularly publishes IHave messages about new gossip.
simple LazyNotifier
{
    parameters:
        double notificationInterval @unit(s);

    gates:
        input gossipInput;
        output notificationOutput;
}
