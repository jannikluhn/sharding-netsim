package sharding.floodsub.flooder;

simple Flooder
{
    gates:
        input sourceInput;
        input gossipInput;
        input addedActivePeerInput;
        input removedActivePeerInput;
        output out;
}
