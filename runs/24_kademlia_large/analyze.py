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
scalar_result_filename_format = "General-{run}.sca"
vector_result_filename_format = "General-{run}.vec"

runs = list(set(range(1)))

PACKET_TYPE_REGEX = r"packetSent-(\d+)-(\d+)-(\d+)"


def plot_peers(results):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    assert len(results) == 1
    vectors = results[0].vectors

    keys = list(vectors.keys())
    node_id_to_keys = {
        int(re.match(r"Network\.nodes\[(\d+)\]", key).group(1)): key
        for key in keys
    }
    node_id_to_vectors = {
        node_id: vectors[key]
        for node_id, key in node_id_to_keys.items()
    }

    xs = []
    ys = []
    labels = []

    for node_id, vector in node_id_to_vectors.items():
        v = vector["peerListUpdate"]
        xs.append(v["time"])
        ys.append(v["value"])
        labels.append(str(node_id))

    # add last peer number to end of data
    x_max = max(xi for x in xs for xi in x)
    for x, y in zip(xs, ys):
        x.append(x_max)
        y.append(y[-1])

    for x, y, label in zip(xs, ys, labels):
        ax.step(x, y, where="post", label=label)
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("Peer number")
    ax.legend()

    return fig


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


if __name__ == "__main__":
    scalar_results = []
    vector_results = []
    for run in runs:
        print(f"loading run {run} of {len(runs)}")
        scalar_path = result_dir / scalar_result_filename_format.format(run=run)
        vector_path = result_dir / vector_result_filename_format.format(run=run)
        #scalar_results.append(load_results(scalar_path))
        vector_results.append(load_results(vector_path))

    peers = plot_peers(vector_results)
    peers.savefig("peers.pdf")
    #load = plot_load(all_results)
    #load.savefig("load.pdf")

    #prop_time = plot_prop_time(all_results)
    #prop_time.savefig("prop_time.pdf")
