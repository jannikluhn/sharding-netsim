package sharding.episub.missing_tracker;


// Module that tracks which messages we are missing and regularly requests them
simple MissingTracker
{
    parameters:
        string cachePath;
        double requestWaitTime @unit(s);
        double requestRoundTripBound @unit(s);

    gates:
        input iHaveInput;
        output out;
}
