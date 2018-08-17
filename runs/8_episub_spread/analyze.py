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
result_filename_format = "EpiSub-10Hz_#2f_1000-#{run}.sca"

runs = list(set(range(50)) - {7, 8, 9, 31, 32, 33, 34})


results = {
    "episub": {
        run: load_results(result_dir / result_filename_format.format(run=run))
        for run in runs
    }
}


def plot_spread(result):
    fig = plt.figure()
    ax = fig.subplots(1, 1)

    values = []
    for run in runs:
        hop_data = get_in(
            ["episub", run, "Network", "newGossipReceived"],
            results
        )
        hist, bins = hop_data["histogram"]

        assert (bins[0], bins[1], bins[-1]) == (-math.inf, 0, math.inf)
        assert np.isclose(hist[0], 0)
        bins = bins[1:-1]  # remove bin edges at infinity
        hist = hist[1:]  # remove negative hops

        cum_hist = np.cumsum(hist) / np.sum(hist)
        values.append(np.argmax(cum_hist > 0.99))

    #main_hist, main_bins = np.histogram(values)
    #ax.bar(main_bins[:-1], main_hist)

    print(list(zip(runs, values)))
    ax.scatter(values, [0] * len(values))
    return fig


if __name__ == "__main__":
    spread = plot_spread(results)
    spread.savefig("spread.pdf")
