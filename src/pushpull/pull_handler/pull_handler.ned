package sharding.pushpull.pull_handler;


simple PullHandler
{
    parameters:
        string cachePath;

    gates:
        input pullInput;
        output out;
};
