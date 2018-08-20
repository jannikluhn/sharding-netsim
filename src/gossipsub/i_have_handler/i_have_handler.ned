package sharding.gossipsub.i_have_handler;

simple IHaveHandler
{
    parameters:
        string cachePath;

    gates:
        input iHaveInput;
        output out;
}
