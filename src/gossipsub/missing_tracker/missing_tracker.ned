package sharding.gossipsub.missing_tracker;


// Module that tracks which messages we are missing and regularly requests them
simple MissingTracker
{
    gates:
        input iHaveInput;
        inout cacheQueryPort;
        output out;
}
