#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse
import os.path
import subprocess
import sys
import tempfile
import time

from lib.cli import (
    TerminalSizeHack,
    add_argument_groups,
    regretful_epilog,
    use_color,
)

from lib.fancy import (
    Reporter,
)

from lib.history import (
    History,
)

from lib.manifest import (
    InvalidManifestError,
    ManifestLoader,
)

from lib.runner import (
    BenchmarkRunner,
    CollectionRunner,
    Constraints,
    Failure,
    Result,
    TimeoutFailure,
)


def main(args):
    ap = argparse.ArgumentParser(
        prog='micro-driver',
        usage="%(prog)s -M MANIFEST -D DIR ... [OPTION ...] [--] [NAME ...]",
        description=(
            "Run micro-benchmarks for the compiler's core library. "
          + " Micro-benchmarks are small programs that execute a certain"
          + " component of the core library and measure its performance. "
          + " This script only orchestrates those programs; it doesn't do any"
          + " timings or statistics on its own."
        ),
        epilog=regretful_epilog,
        add_help=False,
    )
    add_argument_groups(ap)
    with TerminalSizeHack():
        ns = ap.parse_args(args)
    reporter = Reporter(use_color(ns.color), unit='ns')
    if ns.alert is not None and ns.history is None:
        print("micro-driver: warning: --alert has no effect without --history", file=sys.stderr)
    constraints = Constraints(
        timeout=ns.timeout,
        repetitions=ns.repetitions,
        quantile=ns.quantile,
        significance=ns.significance,
        warmup=ns.warmup
    )
    if ns.info:
        reporter.print_info()
    try:
        loader = MicroManifestLoader()
        config = loader.load(ns.manifest)
        with History(ns.history, create=ns.update) as histo:
            mcr = MicroCollectionRunner(
                ns.directories, constraints=constraints,
                logger=lambda m : reporter.print_notice(m) if ns.verbose else None
            )
            return mcr.run_collection(
                selection=ns.benchmarks, config=config, histo=histo,
                update=ns.update, report=reporter, alert=ns.alert
            )
    except (AssertionError, TypeError):
        raise
    except KeyboardInterrupt:
        reporter.print_error("Canceled by keyboard interrupt")
        return 128 + 2
    except Exception as e:
        reporter.print_error(str(e))
        return 1


class MicroManifestLoader(ManifestLoader):

    def _validate_stanza(self, name, definition):
        if 'command' not in definition:
            raise InvalidManifestError(name + ": The 'command' attribute is required")
        for (key, value) in definition.items():
            if key == 'description':
                if type(value) is not str:
                    raise InvalidManifestError(name + "." + key + ": Expected a string")
            elif key == 'command':
                if type(value) is not list or not all(type(s) is str for s in value):
                    raise InvalidManifestError(name + "." + key + ": Expected an array of strings")
                if not value or not value[0].strip():
                    raise InvalidManifestError(name + "." + key + ": Command cannot be empty")
            elif type(key) is str:
                raise InvalidManifestError(name + "." + key + ": Unknown attribute")
            else:
                raise InvalidManifestError()


class MicroCollectionRunner(CollectionRunner):

    def __init__(self, directories, constraints=None, logger=None):
        super().__init__(directories, constraints=constraints, logger=logger)

    def _run_single(self, name, stanza):
        runner = MicroBenchmarkRunner(
            stanza, directories=self.directories, constraints=self.constraints,
            logger=self.logger
        )
        return runner.run()


class MicroBenchmarkRunner(BenchmarkRunner):

    def __init__(self, stanza, directories=None, constraints=None, logger=None):
        super().__init__(directories, constraints=constraints, logger=logger)
        self.__cmd = list()
        try:
            self.__cmd.append(self.find_file(stanza['command'][0]))
            self.__cmd.extend(stanza['command'][1 :])
            args = stanza['command'][0]
        except KeyError as e:
            raise AssertionError(e)

    def run(self):
        try:
            self.logger("Running " + repr(self.__cmd) + " and capturing stdout")
            proc = subprocess.Popen(
                self.__cmd,
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                env=self.constraints.as_environment(os.environ)
            )
            try:
                (stdout, stderr) = proc.communicate(timeout=self.constraints.timeout)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.communicate()
                raise TimeoutFailure(self.constraints.timeout)
        except OSError as e:
            raise Failure(e)
        if proc.returncode != 0:
            raise Failure("Benchmark exited with error code {:d}".format(proc.returncode))
        try:
            return Result.from_string(stdout.decode())
        except ValueError as e:
            raise Failure(e)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
