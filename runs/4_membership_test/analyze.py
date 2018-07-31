import numpy as np
import pandas as pd
import matplotlib as mpl
mpl.use("PDF")
import matplotlib.pyplot as plt


DATA_FILENAME = "results/General-#0.vec"
OUTPUT_FILENAME = "view_sizes.pdf"
SIM_TIME = 600.
NODE_COUNT = 100


def load_data(filename):
    df = pd.read_csv(
        filename,
        skiprows=29 + 2 * NODE_COUNT,
        delim_whitespace=True,
        header=None,
        names=["index", "event", "time", "size"],
    )

    passive_df = df[df["index"] < NODE_COUNT]
    passive_df = passive_df.assign(node=passive_df["index"])
    passive_df.drop(["index", "event"], axis=1, inplace=True)

    active_df = df[df["index"] >= NODE_COUNT]
    active_df = active_df.assign(node=active_df["index"] - NODE_COUNT)
    active_df.drop(["index", "event"], axis=1, inplace=True)

    # add initial and final values
    pre = pd.DataFrame({
        "time": 0,
        "size": 0,
        "node": active_df["node"],
    })
    active_last_size = active_df.groupby("node")["size"].last()
    post_active = pd.DataFrame({
        "time": SIM_TIME,
        "size": active_last_size,
        "node": active_last_size.index,
    })
    passive_last_size = passive_df.groupby("node")["size"].last()
    post_passive = pd.DataFrame({
        "time": SIM_TIME,
        "size": passive_last_size,
        "node": passive_last_size.index,
    })

    active_df = pd.concat([pre, active_df, post_active])
    active_df.reset_index()

    passive_df = pd.concat([pre, passive_df, post_passive])
    passive_df.reset_index()

    return active_df, passive_df


def plot(fig, active_df, passive_df):
    # plot number of connections over time for first n nodes (includes bootstrap nodes)
    ax1 = fig.add_subplot(211)

    colormap = mpl.cm.get_cmap('Set1')
    legend_handles = []
    for i in range(10):
        node = i
        active_sample = active_df[active_df["node"] == node]
        legend_handles.append(ax1.plot(
            active_sample["time"],
            active_sample["size"],
            color=colormap(i),
            linestyle="-",
        )[0])

        passive_sample = passive_df[passive_df["node"] == node]
        legend_handles.append(ax1.plot(
            passive_sample["time"],
            passive_sample["size"],
            color=colormap(i),
            linestyle="--",
        )[0])

    ax1.legend(legend_handles[:2], ["Active view size", "Passive view size"])
    ax1.set_xlabel("Time [s]")
    ax1.set_ylabel("View size")


    ax2 = fig.add_subplot(212)
    # plot histogram of connections at the end of the connection process
    active_connections = active_df.groupby("node")["size"].last()
    active_hist, bins = np.histogram(active_connections, bins=100)

    passive_connections = passive_df.groupby("node")["size"].last()
    passive_hist, passive_bins = np.histogram(passive_connections, bins=100)

    ax2.bar(
        bins[:-1],
        active_hist,
        width=1,
    )
    ax2.set_xlabel("Active peers")
    ax2.set_ylabel("Number of nodes")

if __name__ == "__main__":
    active_df, passive_df = load_data(DATA_FILENAME)
    fig = plt.figure()
    plot(fig, active_df, passive_df)
    plt.savefig(OUTPUT_FILENAME)
