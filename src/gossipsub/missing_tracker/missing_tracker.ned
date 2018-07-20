package sharding.gossipsub.missing_tracker;


// Module that tracks which messages we are missing and regularly requests them
simple MissingTracker
{
    parameters:
        double waitTime @unit(s);

    gates:
        input iHaveInput;
        inout cacheQueryPort;
        output out;
}
