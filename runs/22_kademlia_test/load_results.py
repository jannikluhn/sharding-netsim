from dataclasses import (
    dataclass,
    field,
)
from itertools import (
    chain,
)
import datetime
import math
from pathlib import Path
import re

from typing import (
    Any,
    Dict,
    List,
    Tuple,
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

scalar_regex = re.compile(r"scalar (\S+) (\S+):(\S+) (\S+)$")
field_regex = re.compile(r"field (\S+) (\S+)$")
itervar_regex = re.compile(r"itervar (\S+) (.+)$")
version_regex = re.compile(r"version (\d+)$")
run_regex = re.compile(r"run (\S+)$")
attribute_regex = re.compile(r"attr (\S+) (.+)$")
param_regex = re.compile(r"param (\S+) (.+)$")
histogram_regex = re.compile(r"statistic (\S+) (\S+):histogram$")
vector_regex = re.compile(r"vector (\d+) (\S+) (\S+):vector ([ETV]*)$")
vector_data_regex = re.compile(r"(\d+)\s+(.+)$")
bin_regex = re.compile("bin\s+(\S+)\s+(\S+)$")


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
    histograms: Dict[str, Dict[str, Tuple[List[float], List[int]]]] = field(default_factory=dict)
    vectors: Dict[str, Dict[str, Dict[str, List[Any]]]] = field(default_factory=dict)


def read_header_section(lines):
    version_line = next(lines)
    version_match = version_regex.match(version_line)
    version = int(version_match.group(1))
    if version != 2:
        raise ValueError("unknown version")

    run_line = next(lines)
    run_match = run_regex.match(run_line)
    run = run_match.group(1)

    return {
        "version": version,
        "run": run,
    }, lines


def read_attr_section(lines):
    attributes = {}
    for line in lines:
        attribute_match = attribute_regex.match(line)
        if not attribute_match:
            return attributes, chain([line], lines)
        attribute_name, value = attribute_match.groups()

        if attribute_name in attributes:
            raise ValueError(f"Encountered duplicate attribute {attribute}")
        if attribute_name not in ATTRIBUTES:
            raise ValueError(f"Encountered unknown attribute {attribute}")

        # convert to int or datetime if necessary
        if attribute_name in INT_ATTRIBUTES:
            value = int(value)
        elif attribute_name in DATETIME_ATTRIBUTES:
            value = datetime.datetime.strptime(value, "%Y%m%d-%H:%M:%S")
        else:
            assert attribute_name in STR_ATTRIBUTES

        attributes[attribute_name] = value
    else:
        return attributes, []


def read_itervar_section(lines):

    itervars = {}
    for line in lines:
        match = re.match(itervar_regex, line)
        if not match:
            return itervars, chain([line], lines)

        var, value = match.groups()
        if var in itervars:
            raise ValueError(f"Encountered duplicate itervar {var}")

        itervars[var] = value
    else:
        return itervars, []


def read_param_section(lines):
    params = {}
    for line in lines:
        match = param_regex.match(line)
        if not match:
            return params, chain([line], lines)

        param, value = match.groups()
        if param in params:
            raise ValueError(f"Encountered duplicate param {param}")

        params[param] = value
    else:
        return params, []


def read_content_section(lines):
    scalars = {}
    histograms = {}
    vector_definitions = {}
    vectors_by_index = {}

    while lines:
        # scalars
        scalar, lines = read_scalar(lines)
        if scalar is not None:
            emitter, signal, recorder, value = scalar
            scalars = assoc_in(scalars, [emitter, signal, recorder], value)
            continue

        # histograms
        histogram, lines = read_histogram(lines)
        if histogram is not None:
            emitter, signal, fields, bins, hist = histogram
            for field_name, field_value in fields.items():
                scalars = assoc_in(scalars, [emitter, signal, field_name], field_value)
                histograms = assoc_in(histograms, [emitter, signal], (bins, hist))
            continue

        # vector definitions
        vector, lines = read_vector(lines)
        if vector is not None:
            index, emitter, signal, column_spec = vector
            vector_definitions[index] = (emitter, signal, column_spec)
            vectors_by_index[index] = {"event": [], "time": [], "value": []}
            continue

        # vector data
        vector_data, lines = read_vector_data(lines)
        if vector_data is not None:
            index, column_string = vector_data
            columns = column_string.split()

            if index not in vector_definitions:
                raise ValueError(f"Missing definition for vector {index}")
            _, _, column_spec = vector_definitions[index]

            event, time, value = parse_vector_columns(columns, column_spec)
            v = vectors_by_index[index]
            v["event"].append(event)
            v["time"].append(time)
            v["value"].append(value)
            continue

        # something should have matched by now, if not we're finished
        break

    vectors = {}
    for i, v in vectors_by_index.items():
        emitter, signal, _ = vector_definitions[i]
        vectors = assoc_in(vectors, (emitter, signal), v)

    return (scalars, histograms, vectors), lines


def read_scalar(lines):
    line = next(lines)
    match = scalar_regex.match(line)
    if not match:
        return None, chain([line], lines)
    emitter, signal, recorder, value = match.groups()
    return (emitter, signal, recorder, parse_value(value)), lines


def read_vector(lines):
    line = next(lines)
    match = vector_regex.match(line)
    if not match:
        return None, chain([line], lines)
    index, emitter, signal, column_spec = match.groups()
    return (index, emitter, signal, column_spec), lines


def read_vector_data(lines):
    line = next(lines)
    match = vector_data_regex.match(line)
    if not match:
        return None, chain([line], lines)
    index, column_string = match.groups()
    return (index, column_string), lines


def read_histogram(lines):
    line = next(lines)
    match = histogram_regex.match(line)
    if not match:
        return None, chain([line], lines)
    emitter, signal = match.groups()

    finished = False

    # fields
    fields = {}
    for line in lines:
        field_match = field_regex.match(line)
        if not field_match:
            break

        recorder, value = field_match.groups()
        fields[recorder] = parse_value(value)
    else:
        finished = True

    bins = []
    hist = []
    for line in chain([line], lines):
        bin_match = bin_regex.match(line)
        if not bin_match:
            break

        left_bin_edge, count = bin_match.groups()
        bins.append(float(left_bin_edge))
        hist.append(int(count))
    else:
        finished = True

    result = (emitter, signal, fields, bins, hist)

    if finished:
        return result, []
    else:
        return result, chain([line], lines)


def load_results(path):
    results_dict = {}

    with path.open() as f:
        lines = f
        header, lines = read_header_section(lines)
        attributes, lines = read_attr_section(lines)
        itervars, lines = read_itervar_section(lines)
        params, lines = read_param_section(lines)

        # expect empty line after params section
        param_section_termination_line = next(lines)
        if param_section_termination_line != "\n":
            raise ValueError("Line after param section is not empty")

        (scalars, histograms, vectors), lines = read_content_section(lines)

        # expect empty line at end of file
        file_termination_line = next(lines)
        try:
            next(lines)
        except StopIteration:
            pass
        else:
            raise ValueError("Unexpected lines")
        if file_termination_line != "\n":
            raise ValueError("Last line in file is not empty")

    return Results(
        params=params,
        itervars=itervars,
        scalars=scalars,
        histograms=histograms,
        vectors=vectors,
        **header,
        **attributes,
    )


def parse_value(s):
    try:
        return int(s)
    except ValueError:
        return float(s)
    except ValueError:
        return s


def parse_vector_columns(columns, column_spec):
    columns_parsed = [parse_value(s) for s in columns]
    if "E" in column_spec:
        event = columns_parsed[column_spec.index("E")]
    else:
        event = None
    if "T" in column_spec:
        time = columns_parsed[column_spec.index("T")]
    else:
        time = None
    if "V" in column_spec:
        value = columns_parsed[column_spec.index("V")]
    else:
        value = None
    return event, time, value

