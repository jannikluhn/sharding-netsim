from dataclasses import (
    dataclass,
    field,
)
import datetime
import math
from pathlib import Path
import re

from typing import (
    Any,
    Dict,
)

import numpy as np

from toolz import (
    assoc_in,
    drop,
)


STR_ATTRIBUTES = [
    "configname",
    "experiment",
    "inifile",
    "iterationvars",
    "iterationvarsf",
    "measurement",
    "network",
    "replication",
    "resultdir",
]

INT_ATTRIBUTES = [
    "processid",
    "repetition",
    "runnumber",
    "seedset",
]

DATETIME_ATTRIBUTES = [
    "datetime",
]

ATTRIBUTES = STR_ATTRIBUTES + INT_ATTRIBUTES + DATETIME_ATTRIBUTES

HISTOGRAM_FIELDS = [
    "count",
    "mean",
    "stddev",
    "min",
    "max",
    "sum",
    "sqrsum",
]


@dataclass
class Results:
    version: int
    run: str
    configname: str
    datetime: datetime.datetime
    experiment: str
    inifile: str
    iterationvars: str
    iterationvarsf: str
    measurement: str
    network: str
    processid: int
    repetition: int
    replication: str
    resultdir: Path
    runnumber: int
    seedset: int

    itervars: Dict[str, Any] = field(default_factory=dict)
    params: Dict[str, Any] = field(default_factory=dict)
    scalars: Dict[str, Dict[str, Dict[str, Any]]] = field(default_factory=dict)


def load_results(path):
    results_dict = {}

    with path.open() as f:
        # version
        version_line = next(f)
        version_match = re.match(r"version (\d+)$", version_line)
        version = int(version_match.group(1))
        if version != 2:
            raise ValueError("unknown version")
        results_dict["version"] = version

        # run
        run_line = next(f)
        run_match = re.match(r"run (\S+)$", run_line)
        results_dict["run"] = run_match.group(1)

        # attributes
        found_attributes = set()
        for _ in range(len(ATTRIBUTES)):
            attr_line = next(f)
            attr_match = re.match(r"attr (\S+) (.+)$", attr_line)
            attribute, value = attr_match.groups()
            assert attribute not in found_attributes
            found_attributes.add(attribute)

            if attribute in INT_ATTRIBUTES:
                value = int(value)
            elif attribute in DATETIME_ATTRIBUTES:
                value = datetime.datetime.strptime(value, "%Y%m%d-%H:%M:%S")

            results_dict[attribute] = value
        assert found_attributes == set(ATTRIBUTES)

        # itervars
        l = next(f)
        itervars = {}
        itervar_regex = r"itervar (\S+) (\S+)$"
        match = re.match(itervar_regex, l)
        while match:
            var, value = match.groups()
            assert var not in itervars
            itervars[var] = value
            l = next(f)
            match = re.match(itervar_regex, l)
        results_dict["itervars"] = itervars

        # params
        params = {}
        param_regex = r"param (\S+) (\S+)$"
        match = re.match(param_regex, l)
        while match:
            param, value = match.groups()
            assert param not in params
            params[param] = value
            l = next(f)
            match = re.match(param_regex, l)
        results_dict["params"] = params

        # scalars
        scalars = {}
        assert l == "\n"
        l = next(f)
        scalar_regex = "scalar (\S+) (\S+):(\S+) (\S+)$"
        statistic_regex = "statistic (\S+) (\S+):histogram$"
        field_regex = "field (\S+) (\S+)$"
        bin_regex = "bin\s+(\S+)\s+(\S+)$"
        while True:
            scalar_match = re.match(scalar_regex, l)
            statistic_match = re.match(statistic_regex, l)

            if scalar_match:
                emitter, signal, recorder, value = scalar_match.groups()
                scalars = assoc_in(scalars, [emitter, signal, recorder], parse_value(value))
                l = next(f)
                continue

            elif statistic_match:
                # histogram fields
                emitter, signal = statistic_match.groups()
                found_fields = set()
                for _ in range(len(HISTOGRAM_FIELDS)):
                    l = next(f)
                    field_match = re.match(field_regex, l)
                    recorder, value = field_match.groups()
                    found_fields.add(recorder)
                    scalars = assoc_in(scalars, [emitter, signal, recorder], parse_value(value))
                assert found_fields == set(HISTOGRAM_FIELDS)

                # histogram bins
                bin_edges = []
                hist = []
                l = next(f)
                bin_match = re.match(bin_regex, l)
                while bin_match:
                    left_bin_edge, count = bin_match.groups()
                    bin_edges.append(float(left_bin_edge))
                    hist.append(int(count))
                    l = next(f)
                    bin_match = re.match(bin_regex, l)
                bin_edges.append(math.inf)
                scalars = assoc_in(
                    scalars,
                    [emitter, signal, "histogram"],
                    (np.array(hist), np.array(bin_edges))
                )

            else:
                assert l == "\n"
                try:
                    next(f)
                except StopIteration:
                    break
                else:
                    assert False
    
    
        results_dict["scalars"] = scalars
    return Results(**results_dict)


def parse_value(s):
    try:
        return int(s)
    except ValueError:
        return float(s)
    except ValueError:
        return s
