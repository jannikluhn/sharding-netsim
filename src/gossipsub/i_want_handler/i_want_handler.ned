package sharding.gossipsub.i_want_handler;

simple IWantHandler
{
    parameters:
        string cachePath;

    gates:
        input iWantInput;
        output out;
}
