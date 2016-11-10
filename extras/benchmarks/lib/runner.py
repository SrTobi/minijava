#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import datetime as _datetime
import math as _math
import os.path as _ospath
import time as _time


class CollectionRunner(object):

    def __init__(self, directories, constraints=None, logger=None):
        assert type(directories) is list and all(type(d) is str for d in directories)
        assert constraints is not None
        self.directories = directories
        self.constraints = constraints
        self.logger = logger if logger is not None else lambda x : None

    def run_collection(self, selection=None, config=None, histo=None,
                       update=False, report=None, alert=None, constraints=None):
        assert None not in [config, histo, report]
        if not selection:
            selection = sorted(config.keys())
        successes = set()
        failures = set()
        alerts = set()
        report.print_prolog("Running suite of {:d} benchmarks ...".format(len(selection)))
        if len(selection) > len(set(selection)):
            report.print_warning("List of benchmarks to run contains duplicates")
        t0 = _time.time()
        report.print_header()
        for key in selection:
            try:
                bench = config[key]
            except KeyError:
                report.print_error("No definition for this benchmark in the manifest file", name=key)
                failures.add(key)
                continue
            description = bench.get('description')
            try:
                res = self._run_single(key, bench)
                successes.add(key)
            except Failure as e:
                failures.add(key)
                report.print_error(str(e), name=key)
                continue
            baseline = histo.get_best(key)
            if update:
                histo.register(key, description)
                histo.append(key, res.mean, res.stdev, res.n)
            trend = _get_trend((res.mean, res.stdev), baseline)
            alerted = False if None in [alert, trend] else (trend >= alert)
            if alerted:
                alerts.add(key)
            report.print_row(key, res, trend=trend, alerted=alerted, description=description)
            if res.reason is not None:
                report.print_warning(res.reason, name=key)
        report.print_footer()
        t1 = _time.time()
        elapsed = _datetime.timedelta(seconds=_math.ceil(t1 - t0 + 1.0))
        report.print_epilog("Completed run of benchmark suite in " + str(elapsed))
        report.print_epilog("")
        report.print_epilog("{:6d} successful completions".format(len(successes)))
        report.print_epilog("{:6d} hard failures".format(len(failures)))
        if histo is not None and alert is not None:
            report.print_epilog("{:6d} regression alerts (threshold was {:.2f} sigma)".format(
                len(alerts), alert,
            ))
        report.print_epilog("")
        return len(failures | alerts)

    def _run_single(self, name, stanza):
        """
        @brief
            Runs a single benchmark given its definition from the manifest
            file.

        Concrete classes should override this method to extract the benchmark
        definition from the `stanza` (which is the JSON node in the manifest
        file) and attempt to run the benchmark.

        @param name : str
            unique name of the benchmark (from the manifest file)

        @param stanza : dict
            definition of the benchmark (from the manifest file)

        @returns Result
            result of the benchmark

        @raises Failure
            if no meaningful result could be obtained

        """
        raise NotImplementedError('lib.runner.CollectionRunner._run_single')

    def find_file(self, filename):
        """
        @brief
            Finds a file in the search path.

        @param filename : str
            file to find

        @returns : str
            path to the file if found

        @raises Failure
            if the file cannot be found

        """
        return _do_find_file(filename, self.directories, self.logger)


class BenchmarkRunner(object):

    def __init__(self, directories, constraints=None, logger=None):
        assert type(directories) is list and all(type(d) is str for d in directories)
        assert type(constraints) is Constraints
        self.directories = directories
        self.constraints = constraints
        self.logger = logger if logger is not None else lambda x : None

    def run(self):
        """
        @brief
            Runs a single benchmark.

        Concrete classes should override this method to run the benchmark.

        @returns Result
            result of the benchmark

        @raises Failure
            if no meaningful result could be obtained

        """
        raise NotImplementedError('lib.runner.BenchmarkRunner._run_single')

    def find_file(self, filename):
        """
        @brief
            Finds a file in the search path.

        @param filename : str
            file to find

        @returns : str
            path to the file if found

        @raises Failure
            if the file cannot be found

        """
        return _do_find_file(filename, self.directories, self.logger)


class Failure(Exception):

    pass


class TimeoutFailure(Failure):

    def __init__(self, timeout=None):
        super().__init__(
            "Timeout ({:.2f} s) expired before a result could be obtained".format(timeout)
            if timeout is not None else "Timeout expired before a result could be obtained"
        )


class Constraints(object):

    def __init__(self, timeout=None, repetitions=None, quantile=None, significance=None, warmup=None):
        assert timeout is None or type(timeout) is float and timeout > 0.0
        self.__timeout = timeout
        assert repetitions is None or type(repetitions) is int and repetitions > 3
        self.__repetitions = repetitions
        assert type(quantile) is float and 0.0 < quantile <= 1.0
        self.__quantile = quantile
        assert type(significance) is float and significance > 0.0
        self.__significance = significance
        assert type(warmup) is int and warmup >= 0
        self.__warmup = warmup

    @property
    def timeout(self):
        return self.__timeout

    @property
    def repetitions(self):
        return self.__repetitions

    @property
    def quantile(self):
        return self.__quantile

    @property
    def significance(self):
        return self.__significance

    @property
    def warmup(self):
        return self.__warmup

    def as_environment(self, env=None):
        if env is None:
            env = dict()
        if self.__timeout is not None:
            env['BENCHMARK_TIMEOUT'] = '{:.6g}'.format(self.__timeout)
        if self.__repetitions is not None:
            env['BENCHMARK_REPETITIONS'] = '{:d}'.format(self.__repetitions)
        env['BENCHMARK_QUANTILE'] = '{:.6g}'.format(self.__quantile)
        env['BENCHMARK_SIGNIFICANCE'] = '{:.6g}'.format(self.__significance)
        env['BENCHMARK_WARMUP'] = '{:d}'.format(self.__warmup)
        return env


class Result(object):

    def __init__(self, mean, stdev, n, reason=None):
        self.mean = mean
        self.stdev = stdev
        self.n = n
        self.reason = reason

    @classmethod
    def from_string(cls, text):
        try:
            (w0, w1, w2) = text.split()
            mean = float(w0)
            stdev = float(w1)
            n = int(w2)
        except ValueError:
            raise ValueError("Benchmark result string not in format '%g %g %d'")
        return Result(mean, stdev, n)


def _do_find_file(filename, directories, logger):
    logger("Searching for file {} ...".format(repr(filename)))
    for directory in directories:
        path = _ospath.join(directory, filename)
        if _ospath.exists(path):
            logger("  ... {}:  YES".format(repr(path)))
            return path
        else:
            logger("  ... {}:  NO".format(repr(path)))
    raise Failure("Cannot find file: " + filename)


def _get_trend(current, best=None):
    """
    @brief
        Computes the trend of a benchmark compared to its historical best
        result and determines whether this is alerting.

    If the historic best result is unknown (`best` is `None` or a tuple
    containg `None`s), then nothing can be computed, and `(None, False)` is
    `return`ed immediately.

    Otherwise, the difference of the current and the historic best result is
    computed along with the standard deviation of this difference.  Finally,
    the quotient of the different and its standard deviation is `return`ed.

    If any of the computations fails (for example, because the standard
    deviation is zero), `None` is `return`ed.

    @param current : (float, float)
        mean and standard deviation of the current result

    @param best : (float, float) | (NoneType, NoneType) | NoneType
        mean and standard deviation of the historical best result or `None` if
        unknown

    @returns float | NoneType
        the relative trend or `None`if it cannot be computed

    """
    if best is None or None in best:
        return None
    try:
        diff = current[0] - best[0]
        sigma = _math.sqrt(current[1]**2 + best[1]**2)
        return diff / sigma
    except ArithmeticError:
        return None
