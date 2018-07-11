network FloodSub
{
    parameters:
        int numNodes = 10;
        double txRate;

    submodules:
        node[numNodes]: FloodSubNode {
            parameters:
                txRate = txRate;
        }

    connections:
        node[0].port++ <--> { delay = 0.08004094538536666s; } <--> node[3].port++;
        node[0].port++ <--> { delay = 0.08219179952151766s; } <--> node[9].port++;
        node[1].port++ <--> { delay = 0.012184687550062592s; } <--> node[7].port++;
        node[1].port++ <--> { delay = 0.05415124971727703s; } <--> node[4].port++;
        node[1].port++ <--> { delay = 0.11465398191285331s; } <--> node[2].port++;
        node[2].port++ <--> { delay = 0.12264683958671044s; } <--> node[6].port++;
        node[3].port++ <--> { delay = 0.17812093806347257s; } <--> node[4].port++;
        node[4].port++ <--> { delay = 0.11451781561995242s; } <--> node[6].port++;
        node[4].port++ <--> { delay = 0.020235812934631003s; } <--> node[7].port++;
        node[5].port++ <--> { delay = 0.06180015056406085s; } <--> node[6].port++;
        node[5].port++ <--> { delay = 0.0760754368869987s; } <--> node[8].port++;
        node[5].port++ <--> { delay = 0.16221396195288212s; } <--> node[9].port++;
        node[6].port++ <--> { delay = 0.20667426589379256s; } <--> node[8].port++;
}
