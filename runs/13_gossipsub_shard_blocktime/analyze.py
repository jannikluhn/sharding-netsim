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

runs = list(set(range(10)))

PACKET_TYPE_REGEX = r"packetSent-(\d+)-(\d+)-(\d+)"

BLOCK_SIZE = 1 * 1024 * 1024  # MB


def plot_load(all_results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    mesh_degrees = []
    payload_load = []
    control_load = []

    for results in all_results:
        mesh_degree = int(results.itervars["M"])
        hub_scalars = results.scalars["Network.hub"]

        total_gossip = results.scalars["Network"]["newGossipEmitted"]["count"]
        node_count = int(results.itervars["N"])


        hub_keys = list(results.scalars["Network.hub"].keys())
        matches = [re.match(PACKET_TYPE_REGEX, key) for key in hub_keys]
        packets = {
            tuple(int(i) for i in match.groups()): hub_scalars[match.string]["count"]
            for match in matches
            if match
        }

        gossip = packets[(1, 0, 0)]
        non_gossip = sum(
            n for packet_id, n in packets.items()
            if packet_id[0] == 1 and packet_id != (1, 0, 0)
        )

        mesh_degrees.append(mesh_degree)
        payload_load.append(gossip / total_gossip / node_count)
        control_load.append(non_gossip / total_gossip / node_count)

    colormap = mpl.cm.get_cmap('Set1')
    plot_kwargs = {
        "linestyle": "",
        "marker": "x",
    }
    ax.plot(
        mesh_degrees,
        payload_load,
        color=colormap(0),
        label="Payload",
        **plot_kwargs
    )
    ax.plot(
        mesh_degrees,
        control_load,
        color=colormap(1),
        label="Control",
        **plot_kwargs
    )

    ax.legend()
    ax.set_xlabel("Target mesh degree")
    ax.set_ylabel("Messages sent per node and gossip event")

    return fig


def plot_prop_time(all_results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    xs = []
    ys_time = []
    ys_loss = []

    for results in all_results:
        node_count = int(results.itervars["N"])
        mesh_degree = int(results.itervars["M"])
        block_time = 1 / float(results.itervars["R"][:-2])
        hist, bins = get_in(["Network", "newGossipReceived", "histogram"], results.scalars)
        gossip_emitted = get_in(["Network", "newGossipEmitted", "count"], results.scalars)
        gossip_received = get_in(["Network", "newGossipReceived", "count"], results.scalars)

        if get_in(["Network", "newGossipEmitted", "count"], results.scalars) == 0:
            print(f"warning: no messages emitted in run {results.runnumber}")

        xs.append(block_time)

        hops = calc_hops(hist, bins, 0.99)
        ys_time.append(hops)

        loss = calc_loss(gossip_emitted, gossip_received, node_count)
        ys_loss.append(loss)


    colormap = mpl.cm.get_cmap('Set1')
    plot_kwargs = {
        "linestyle": "",
        "marker": "x",
    }
    ax.plot(
        xs,
        ys_time,
        color=colormap(0),
        **plot_kwargs
    )

    ax_loss = ax.twinx()
    ax_loss.plot(
        xs,
        ys_loss,
        color=colormap(1),
        **plot_kwargs
    )
    print(ys_loss)

    ax.set_xlabel("Block time [s]")
    ax.set_ylabel("Propagation time [s]")
    ax.set_title("Time to propagate through >99% of the network")

    return fig


def plot_hists(results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    colormap = mpl.cm.get_cmap('tab20b')
    plot_kwargs = {
    }

    for i, results in enumerate(all_results):
        throughput = BLOCK_SIZE * float(results.itervars["R"][:-2]) 
        hist, bins = get_in(["Network", "newGossipReceived", "histogram"], results.scalars)
        cum_hist = np.cumsum(hist) / np.sum(hist)

        if get_in(["Network", "newGossipEmitted", "count"], results.scalars) == 0:
            print(f"warning: no messages emitted in run {results.runnumber}")

        ax.plot(
            bins[1:],
            cum_hist,
            color=colormap(i),
            label=f"{throughput / 1024:.2f}",
            **plot_kwargs,
        )

    ax.set_xlabel("Propagation time [s]")
    ax.set_ylabel("Propagation progress")
    ax.set_xlim(0, 25)
    ax.legend(title="Throughput [kB/s]")

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

    prop_time = plot_prop_time(all_results)
    prop_time.savefig("prop_time.pdf")

    hists = plot_hists(all_results)
    hists.savefig("hists.pdf")

    check_message_loss(all_results)
