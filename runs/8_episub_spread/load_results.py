import math

import numpy as np

from toolz import (
    assoc_in,
    drop,
)


def parse_value(s):
    try:
        return int(s)
    except ValueError:
        return float(s)


def load_results(path):
    scalars = {}  # {emitter: {signal: {recorder: data}}}
    with path.open() as f:

        # skip header
        for l in f:
            if len(l.strip()) == 0:
                break
        
        l = next(f)
        while True:
            # skip empty lines
            if len(l.strip()) == 0:
                break

            # scalars
            elif l.startswith("scalar"):
                _, emitter, signal_and_recorder, s = l.split()
                signal, recorder = signal_and_recorder.split(":")

                scalars = assoc_in(scalars, [emitter, signal, recorder], parse_value(s))

                try:
                    l = next(f)
                except StopIteration:
                    break

            # statistics
            elif l.startswith("statistic"):
                _, emitter, signal_and_recorder = l.split()
                signal, recorder = signal_and_recorder.split(":")
                assert recorder == "histogram"

                # read statistics
                l = next(f)
                while l.startswith("field"):
                    _, recorder, s = l.split()
                    scalars = assoc_in(scalars, [emitter, signal, recorder], parse_value(s))

                    try:
                        l = next(f)
                    except StopIteration:
                        break

                bin_edges = []
                hist = []

                # read histogram data
                while l.startswith("bin"):
                    _, left_bin_edge, v = l.split()
                    bin_edges.append(float(left_bin_edge))
                    hist.append(float(v))

                    try:
                        l = next(f)
                    except StopIteration:
                        break
                bin_edges.append(math.inf)
                scalars = assoc_in(
                    scalars,
                    [emitter, signal, "histogram"],
                    (np.array(hist), np.array(bin_edges))
                )

            else:
                assert False

    return scalars
