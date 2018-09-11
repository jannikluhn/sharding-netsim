import math
from pathlib import Path
import re

import numpy as np
from scipy.optimize import curve_fit

import matplotlib as mpl
mpl.use("PDF")
import matplotlib.pyplot as plt
from texttable import Texttable

from toolz import (
    get_in,
    valmap,
)

from load_results import load_results


root_dir = Path(__file__).parent
result_dir = root_dir / "results"
result_filename_format = "General-{run}.sca"

runs = list(set(range(6)))

PACKET_TYPE_REGEX = r"packetSent-(\d+)-(\d+)-(\d+)"

BLOCK_SIZE = 1 * 1024 * 1024  # MB


def plot_hists(results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    colormap = mpl.cm.get_cmap("tab20b")
    plot_kwargs = {
    }

    for i, results in enumerate(all_results):
        hist, bins = get_in(["Network", "newGossipReceived", "histogram"], results.scalars)
        cum_hist = np.cumsum(hist) / np.sum(hist)

        if get_in(["Network", "newGossipEmitted", "count"], results.scalars) == 0:
            print(f"warning: no messages emitted in run {results.runnumber}")

        block_size = eval(results.itervars["S"].strip("\""))
        block_time = 1 / eval(results.itervars["R"].rstrip("Hz"))

        print(bins[np.argmax(cum_hist > 0.99)] / block_time, block_time)

        ax.plot(
            bins[1:],
            cum_hist,
            color=colormap(i),
            label=f"{block_size / 1024 / 1024/ 8:0.1f}MB, {block_time:.1f}s",
            **plot_kwargs,
        )

    ax.set_xlabel("Propagation time [s]")
    ax.set_ylabel("Propagation progress")
    ax.set_xlim(0, 20)
    ax.legend(title="Blocks")

    return fig


def plot_channel_occupancy(results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    colormap = mpl.cm.get_cmap("tab20b")
    for i, results in enumerate(all_results):
        node_count = int(results.itervars["N"])
        channel_used_keys = [f"channelUsed-{i}" for i in range(node_count)]
        channel_occupancy = [
            results.scalars["Network.hub"][key]["last(sumPerDuration)"]
            for key in channel_used_keys
        ]

        hist, bins = np.histogram(channel_occupancy, bins=np.linspace(0, 1, 101))

        block_size = eval(results.itervars["S"].strip("\""))
        block_time = 1 / eval(results.itervars["R"].rstrip("Hz"))

        ax.plot(
            bins[1:],
            np.cumsum(hist) / np.sum(hist),
            color=colormap(i),
            label=f"{block_size / 1024 / 1024/ 8:0.1f}MB, {block_time:.1f}s",
        )

    ax.set_xlabel("Channel load")
    ax.set_ylabel("Fraction of nodes")
    ax.legend(title="Blocks")

    return fig


def calc_hops(hist, bins, threshold):
    """Given a hop histogram, calculate the number of hops it takes to reach more than a threshold
    of all nodes.
    """
    # check that bins are `-inf | ? | ? | ... | ? | +inf`
    assert (bins[0], bins[-1]) == (-math.inf, math.inf)

    assert np.isclose(hist[0], 0)
    bins = bins[1:-1]  # remove bin edges at infinity
    hist = hist[1:]  # remove negative hops

    cum_hist = np.cumsum(hist) / np.sum(hist)
    return np.argmax(cum_hist > threshold)


def calc_loss(emitted, received, node_count):
    if emitted == 0:
        return 0
    else:
        return 1 - received / (node_count * emitted)


def check_message_loss(all_results):
    for i, results in enumerate(all_results):
        node_count = int(results.itervars["N"])
        gossip_emitted = results.scalars["Network"]["newGossipEmitted"]["count"]
        gossip_received = results.scalars["Network"]["newGossipReceived"]["count"]
        loss = calc_loss(gossip_emitted, gossip_received, node_count)
        if loss > 0:
            print(f"warning: non-zero loss in run {i}")


if __name__ == "__main__":
    all_results = []
    for run in runs:
        print(f"loading run {run} of {len(runs)}")
        path = result_dir / result_filename_format.format(run=run)
        results = load_results(path)
        all_results.append(results)

    #load = plot_load(all_results)
    #load.savefig("load.pdf")

    #prop_time = plot_prop_time(all_results)
    #prop_time.savefig("prop_time.pdf")

    hists = plot_hists(all_results)
    hists.savefig("hists.pdf")

    occ = plot_channel_occupancy(all_results)
    occ.savefig("occ.pdf")

    check_message_loss(all_results)
