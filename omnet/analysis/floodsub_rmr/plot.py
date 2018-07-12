import pandas as pd
import matplotlib as mpl
mpl.use("PDF")
import matplotlib.pyplot as plt


def plot(ax1, ax2, df):
    df_node_number = df[df["min_peers"] == 5]
    ax1.plot(
        df_node_number["nodes"],
        df_node_number["rmr"],
        linestyle="none",
        marker="x",
    )
    ax1.set_xlabel("Node number")
    ax1.set_ylim(0, 5)
    ax1.set_ylabel("Relative message redundancy")
    ax1.set_title("Min peers: 5")

    df_min_peers = df[df["nodes"] == 100]
    ax2.plot(
        df_min_peers["min_peers"],
        df_min_peers["rmr"],
        linestyle="none",
        marker="x",
    )
    ax2.set_xlabel("Min peers")
    ax2.set_ylim(0, None)
    ax2.set_title("Nodes: 100")


def calc_rmr(transmissions, messages, nodes):
    return transmissions / messages / (nodes - 1) - 1


if __name__ == "__main__":
    df = pd.read_csv("data.csv", delim_whitespace=True)
    df["rmr"] = calc_rmr(df["transmissions"], df["messages"], df["nodes"])

    fig = plt.figure()
    ax1 = fig.add_subplot(121)
    ax2 = fig.add_subplot(122)

    plot(ax1, ax2, df)
    plt.savefig("plot.pdf")
