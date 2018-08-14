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
result_filename_format = "EpiSub-nodeCount={node_count},rampUpTime={node_count}_#2a_1s,10Hz_#2f_{node_count},{node_count}_#2a_1s_+_20s-#{run}.sca"

node_counts = [100, 500, 1000, 5000, 10000, 20000]
runs = {
    100: [0, 1, 2],
    500: [0, 1, 2],
    1000: [0],
    5000: [0, 1, 2],
    10000: [0, 1, 2],
    20000: [0, 1, 2],
}


results = {
    "episub": {
        node_count: [
            load_results(
                result_dir / result_filename_format.format(node_count=node_count, run=run)
            )
            for run in runs[node_count]
        ]
        for node_count in node_counts
    }
}

protocols = [
    {
        "name": "episub",
        "title": "EpiSub",
    },
]


def plot_hop_histograms(results, protocols):
    figures = {}

    hist_kwargs = {
        "width": 0.9,
        "align": "center",
    }

    for protocol in protocols:
        fig = plt.figure()
        figures[protocol["title"]] = fig

        fig.set_tight_layout(True)
        fig.suptitle(protocol["title"])

        n_plots = len(results[protocol["name"]])
        n_cols = 2
        n_rows = (n_plots - 1) // n_cols + 1
        axes_nested = fig.subplots(n_rows, n_cols, squeeze=False)
        axes = [ax for rows in axes_nested for ax in rows]

        for ax, node_count in zip(axes, results[protocol["name"]]):
            hop_data = get_in(
                [protocol["name"], node_count, "Network", "newGossipReceived"],
                results
            )
            hist, bins = hop_data["histogram"]

            assert (bins[0], bins[1], bins[-1]) == (-math.inf, 0, math.inf)
            assert np.isclose(hist[0], 0)
            bins = bins[1:-1]  # remove bin edges at infinity
            hist = hist[1:]  # remove negative hops

            cum_hist = np.cumsum(hist) / np.sum(hist)
            ax.bar(bins, cum_hist, **hist_kwargs)

            mean = hop_data["mean"]
            std = hop_data["stddev"]
            ax.axvline(mean, color="red")

            ax.axvline(np.argmax(cum_hist > 0.99), color="red")

            ax.set_title(f"{node_count} nodes")
            #ax.set_xticks(bins)

        # only label edges of outer plots
        for axes_row in axes_nested:
            axes_row[0].set_ylabel("Frequency")
        for ax in axes_nested[-1]:
            ax.set_xlabel("Hops")

    return figures


def plot_hop_growth(results, protocols):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    colormap = mpl.cm.get_cmap('Set1')
    for i, protocol in enumerate(protocols):
        node_counts = list(results[protocol["name"]].keys())

        x = []
        y = []
        for node_count in node_counts:
            for run in runs[node_count]:
                hop_data = get_in(
                    [protocol["name"], node_count, run, "Network", "newGossipReceived"],
                    results
                )
                hist, bins = hop_data["histogram"]

                assert (bins[0], bins[1], bins[-1]) == (-math.inf, 0, math.inf)
                assert np.isclose(hist[0], 0)
                bins = bins[1:-1]  # remove bin edges at infinity
                hist = hist[1:]  # remove negative hops

                cum_hist = np.cumsum(hist) / np.sum(hist)
                y.append(np.argmax(cum_hist > 0.99))
                x.append(node_count)

        ax.plot(x, y, color=colormap(i), label=protocol["title"], linestyle="", marker="x")

        f = lambda x, a: a * np.log(x)
        p_opt, p_cov = curve_fit(f, x, y, [4])
        x_fit = np.linspace(1, 20000, 100)
        y_fit = f(x_fit, *p_opt)
        ax.plot(
            x_fit,
            y_fit,
            linestyle="-",
            color=colormap(i + 1),
            label="Logarithmic fit"
        )

    ax.legend()
    ax.set_xlabel("Network size")
    ax.set_ylabel("Hops")
    ax.set_title("Hops to reach >99% of the network")

    return fig


def plot_message_efficiency(results, protocols):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    colormap = mpl.cm.get_cmap('Set1')
    for i, protocol in enumerate(protocols):
        node_counts = list(results[protocol["name"]].keys())

        y = []
        for node_count in node_counts:
            data = get_in(
                [protocol["name"], node_count, "Network"],
                results
            )
            gossip_emitted = data["newGossipEmitted"]["count"]
            messages_sent = data["messageSent"]["count"]
            y.append(messages_sent / gossip_emitted / node_count)
            
        ax.plot(node_counts, y, color=colormap(i), label=protocol["title"])

    ax.legend()
    ax.set_xlabel("Network size")
    ax.set_ylabel("Hops")
    ax.set_title("Messages sent per rumor and node")

    return fig


def print_losses(results, protocols):
    table = Texttable()

    node_counts = list(results[protocols[0]["name"]].keys())
    table.set_cols_align(["l", "r", "r", "r"])
    table.add_row(["Protocol", "Node count", "Run #", "Loss"])

    for protocol in protocols:
        for node_count in node_counts:
            for run in runs[node_count]:
                data = get_in(
                    [protocol["name"], node_count, run, "Network"],
                    results
                )
                gossip_emitted = data["newGossipEmitted"]["count"]
                gossip_received = data["newGossipReceived"]["count"]
                loss = 1 - gossip_received / ((node_count - 1) * gossip_emitted)
                table.add_row([protocol["title"], node_count, run, loss])

    print(table.draw())
            



if __name__ == "__main__":
    #hop_histograms = plot_hop_histograms(results, protocols)
    #for title, fig in hop_histograms.items():
    #    fig.savefig(title + ".pdf")

    hop_growth = plot_hop_growth(results, protocols)
    hop_growth.savefig("hop_growth.pdf")

    # Efficiency is misleading here because measurement time >> gossiping time (so lots of
    # discovery overhead)
    #efficiency = plot_message_efficiency(results, protocols)
    #efficiency.savefig("efficiency.pdf")

    print_losses(results, protocols)
