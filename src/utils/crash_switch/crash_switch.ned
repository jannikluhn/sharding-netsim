package sharding.utils.crash_switch;


simple CrashSwitch
{
    parameters:
        volatile bool crashed;

    gates:
        input in;
        input sourceInput;
        output out;
        output sourceOutput;
}
