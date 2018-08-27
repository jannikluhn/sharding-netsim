import math
from pathlib import Path

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

runs = list(set(range(140)))



def calc_hops(hist, bins, threshold):
    """Given a hop histogram, calculate the number of hops it takes to reach more than a threshold
    of all nodes.
    """
    # check that bins are `-inf | 0 | 1 | ... | n | +inf`
    assert (bins[0], bins[-1]) == (-math.inf, math.inf)
    assert (bins[1:-1] == list(range(0, len(bins) - 2))).all()

    assert np.isclose(hist[0], 0)
    bins = bins[1:-1]  # remove bin edges at infinity
    hist = hist[1:]  # remove negative hops

    cum_hist = np.cumsum(hist) / np.sum(hist)
    return np.argmax(cum_hist > threshold)


def plot_hop_growth(all_results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    xs_gossipsub = []
    ys_gossipsub = []
    xs_episub = []
    ys_episub = []

    for results in all_results:
        node_count = int(results.itervars["N"])
        hist, bins = get_in(["Network", "newGossipReceived", "histogram"], results.scalars)

        if get_in(["Network", "newGossipEmitted", "count"], results.scalars) == 0:
            print(f"warning: no messages emitted in run {results.runnumber}")

        hops = calc_hops(hist, bins, 0.99)
        gossiper_module_type = results.params["Network.nodes[*].gossiperType"]

        if gossiper_module_type == "\"\\\"sharding.gossipsub.GossipSubGossiper\\\"\"":
            xs_gossipsub.append(node_count)
            ys_gossipsub.append(hops)
        elif gossiper_module_type == "\"\\\"sharding.episub.EpiSubGossiper\\\"\"":
            xs_episub.append(node_count)
            ys_episub.append(hops)
        else:
            assert False

    colormap = mpl.cm.get_cmap('Set1')
    plot_kwargs = {
        "linestyle": "",
        "marker": "x",
    }
    ax.plot(
        xs_gossipsub,
        ys_gossipsub,
        color=colormap(0),
        label="GossipSub",
        **plot_kwargs
    )
    ax.plot(
        xs_episub,
        ys_episub,
        color=colormap(1),
        label="EpiSub",
        **plot_kwargs
    )

    ax.legend()
    ax.set_xlabel("Network size")
    ax.set_ylabel("Hops")
    ax.set_title("Hops to reach >99% of the network")

    return fig


def check_message_loss(all_results):
    for i, results in enumerate(all_results):
        node_count = int(results.itervars["N"])
        gossip_emitted = results.scalars["Network"]["newGossipEmitted"]["count"]
        gossip_received = results.scalars["Network"]["newGossipReceived"]["count"]
        if gossip_emitted == 0:
            loss = 0
        else:
            loss = 1 - gossip_received / ((node_count - 1) * gossip_emitted)
        if loss > 0:
            print(f"warning: non-zero loss in run {i}")


if __name__ == "__main__":
    all_results = []
    for run in runs:
        print(f"loading run {run} of {len(runs)}")
        path = result_dir / result_filename_format.format(run=run)
        results = load_results(path)
        all_results.append(results)
    hop_growth = plot_hop_growth(all_results)
    hop_growth.savefig("hop_growth.pdf")

    check_message_loss(all_results)
