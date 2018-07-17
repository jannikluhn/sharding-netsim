package sharding.gossipsub.multicaster;


// Module that multicasts messages to a set of receivers.
simple Multicaster
{
    gates:
        input inputs[];
        output out;
        input controlInput;
}
