package sharding.gossipsub.missing_tracker;


// Module that tracks which messages we are missing and regularly requests them
simple MissingTracker
{
    parameters:
        string cachePath;
        double waitTime @unit(s);

        @signal[missingRequested](type=long);

    gates:
        input iHaveInput;
        output out;
}
