import numpy as np
import matplotlib as mpl
mpl.use("PDF")
import matplotlib.pyplot as plt


# from Run1TestLargeNetwork-#0.sca
NODE_COUNT = 100000
GOSSIP_EMITTED = 95
GOSSIP_RECEIVED = 2011847
MESSAGES_SENT = 18579494
HOP_COUNT_HISTOGRAM = np.array([
    0,
    95,
    325,
    1508,
    7068,
    31436,
    106838,
    249922,
    401683,
    458714,
    380132,
    230818,
    102203,
    32528,
    7230,
    1186,
    153,
    8,
    0,
])

RELIABILITY = GOSSIP_RECEIVED / (GOSSIP_EMITTED * NODE_COUNT)
RELATIVE_MESSAGE_REDUNDANCY = MESSAGES_SENT / (RELIABILITY * NODE_COUNT - 1) - 1


def plot(ax):
    xs = np.arange(len(HOP_COUNT_HISTOGRAM))
    ax1 = ax
    ax2 = ax.twinx()

    ax1.bar(
        xs,
        HOP_COUNT_HISTOGRAM / np.sum(HOP_COUNT_HISTOGRAM),
        width=1,
    )
    ax2.plot(
        xs,
        np.cumsum(HOP_COUNT_HISTOGRAM / np.sum(HOP_COUNT_HISTOGRAM)),
        color="red",
    )
    ax1.set_xlabel("Hop count")
    ax1.set_ylabel("Frequency")
    ax2.set_ylabel("Cumulative")
    ax2.set_ylim(0, 1)
    ax1.set_xticks(np.arange(len(HOP_COUNT_HISTOGRAM), step=2))

if __name__ == "__main__":
    print(f"Node count: {NODE_COUNT}")
    print(f"Reliability: {RELIABILITY}")
    print(f"RMR: {RELATIVE_MESSAGE_REDUNDANCY}")

    fig = plt.figure()
    ax = fig.add_subplot(111)
    plot(ax)
    plt.savefig("hop_count_histogram.pdf")
