#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse
import os.path
import sys
import time

from lib.cli import (
    TerminalSizeHack,
    add_help,
    add_history_mandatory,
)

from lib.history import (
    History,
)


def main(args):
    ap = argparse.ArgumentParser(
        prog='history',
        usage="%(prog)s -H DATABASE ACTION",
        description="Manage the history database of benchmark results.",
        allow_abbrev=False,
        add_help=False
    )
    actions = ap.add_argument_group(
        title="Action Parameters",
        description=(
              "These parameters select the action to perform on the database."
            + " Exactly one of them must be used.  Since executing the wrong"
            + " action by accident could potentially have bad consequences,"
            + " these parameters have no short forms to avoid bad surprises."
        )
    )
    mandatory = ap.add_argument_group(title="Mandatory Parameters", description="")
    supplementary = ap.add_argument_group(title="Supplementary Parameters", description="")
    add_history_mandatory(mandatory)
    add_help(supplementary)
    group = actions.add_mutually_exclusive_group(required=True)
    group.add_argument(
        '--list', action='store_true',
        help=(
              "List all benchmarks in the database with their names and"
            + " optional descriptions."
        )
    )
    group.add_argument(
        '--export', metavar='NAME',
        help=(
              "Export history data for benchmark NAME in a text format that"
            + " can, for example, be given to Gnuplot or some other"
            + " data-processing software."
        ),
    )
    group.add_argument(
        '--drop', metavar='NAME',
        help=(
              "Remove all data for benchmark NAME irrecoverably from the"
            + " database."
        )
    )
    group.add_argument(
        '--drop-since', metavar='TIME', type=int,
        help=(
              "Irrecoverably remove all results since TIME from the database. "
            + " TIME must be a POSIX time-stamp.  You can use the 'date'"
            + " command-line utility to translate human-friendly notions of"
            + " time-points into POSIX time-stamps."
        ),
    )
    with TerminalSizeHack():
        ns = ap.parse_args(args)
    with History(ns.history) as histo:
        if not histo:
            raise RuntimeError("Database does not exist")
        if ns.list:
            _action_list(histo)
        if ns.export is not None:
            _action_export(histo, ns.export)
        if ns.drop is not None:
            histo.drop_benchmark(ns.drop)
        if ns.drop_since is not None:
            histo.drop_since(ns.drop_since)


def _action_list(histo):
    summary = histo.get_descriptions()
    for name in sorted(summary.keys()):
        desc = summary[name]
        if desc is None:
            print(name)
        else:
            print('{:20s}  {:s}'.format(name, desc))


def _action_export(histo, name):
    bench = histo.get_benchmark_results(name)
    printhdr = lambda k, v : print('# {:22s}{}'.format(k + ':', v))
    printhdr("Benchmark Name", name)
    if bench.description is not None:
        printhdr("Description", bench.description)
    printhdr("No. of Data-Points", len(bench.results))
    if bench.results:
        timefmt = '%a, %d %b %Y %H:%M:%S %z'  # RFC 2822
        first = time.localtime(min(hr.timestamp for hr in bench.results))
        last = time.localtime(max(hr.timestamp for hr in bench.results))
        printhdr("First Data-Point", time.strftime(timefmt, first))
        printhdr("Last Data-Point", time.strftime(timefmt, last))
    print()
    print('# {:>14s}{:>16s}{:>16s}{:>16s}'.format("timestamp", "mean / s", "stdev / s", "N"))
    print()
    for hr in sorted(bench.results, key=lambda r : r.timestamp):
        print('{:16d}{:16.3e}{:16.3e}{:16d}'.format(
            hr.timestamp, hr.mean, hr.stdev, hr.n
        ))


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except (AssertionError, TypeError):
        raise
    except Exception as e:
        print("error:", e, file=sys.stderr)
        sys.exit(1)
