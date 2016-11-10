#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse as _argparse
import os as _os
import shutil as _shutil
import sys as _sys


class TerminalSizeHack(object):

    """
    @brief
        A context manager to temporairly set the `LINES` and `COLUMNS`
        variables in `os.environ` to the current terminal size.

    The `argparse` module from the standard library doesn't determine the
    terminal size very wisely.  Wrapping the call to the `parse_args` method of
    `argparse.ArgumentParser` inside a context guarded by this `class` will
    cause it to use the correct terminal size.

    This issue was alread reported in 2011 but the path apparently hasn't found
    its way into the standard library yet.

        https://bugs.python.org/issue13041

    """

    def __init__(self):
        self.__lines = None
        self.__columns = None

    def __enter__(self):
        self.__lines = _os.environ.get('LINES')
        self.__columns = _os.environ.get('COLUMNS')
        size = _shutil.get_terminal_size()
        _os.environ['LINES'] = str(size.lines)
        _os.environ['COLUMNS'] = str(size.columns)

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.__lines is None:
            del _os.environ['LINES']
        else:
            _os.environ['LINES'] = self.__lines
        if self.__columns is None:
            del _os.environ['COLUMNS']
        else:
            _os.environ['COLUMNS'] = self.__columns


regretful_epilog = """
The command-line interface of this script is optimized for generality, not for
quick use.  If you plan to run it on a regular basis, you probably want to
create a small wrapper script that invokes it with the arguments that are
appropriate for your intents and purposes.
"""


def add_argument_groups(ap):
    pos = ap.add_argument_group(title="Positional Aruments", description="")
    ess = ap.add_argument_group(title="Essential Options", description="")
    sta = ap.add_argument_group(title="Statistical Options", description="")
    sup = ap.add_argument_group(title="Supplementary Options", description="")
    add_benchmarks(pos)
    add_manifest(ess)
    add_directories(ess)
    add_constraints(sta)
    add_history_optional(sta)
    add_alert(sta)
    add_info(sup)
    add_verbose(sup)
    add_color(sup)
    add_help(sup)
    return (pos, ess, sta, sup)


def add_benchmarks(arg):
    arg.add_argument(
        'benchmarks', metavar='NAME', nargs='*',
        help=(
              "Run benchmarks for NAMEs.  If no benchmarks are selected this"
            + " way, then the entire suite will be run."
        )
    )


def add_manifest(arg):
    arg.add_argument(
        '-M', '--manifest', metavar='FILE', required=True,
        type=_ArgInput(),
        help=(
              "Read benchmark descriptions from the Json manifest in FILE."
            + " The special file-name '-' can be used to read from standard"
            + " input."
        )
    )


def add_directories(arg):
    arg.add_argument(
        '-D', '--directories', metavar='DIR', nargs='+', default=['.'],
        type=_ArgDirectory(),
        help=(
               "Directories to search for input files and generators.  The"
            + " directories will be searched in the order they are mentioned. "
            + " If this option is not used, only the current working directory"
            + " '.' will be searched for files.  Note however that if you do"
            + " use this option and also want to search '.', you have to"
            + " mention it explicitly as it will not be added automatically"
            + " in this case.  This is so you can always have the last word"
            + " and decide what directories to include and in which order."
        )
    )


def add_history_optional(arg):
    arg.add_argument(
        '-H', '--history', metavar='FILE', default=None,
        type=_ArgRealFile(existing=False),
        help=(
              "Use the history database in FILE which must be a readable"
            + " and writeable file.  If it does not exist, it will be"
            + " created.  The default if this option is not given is to not"
            + " use a history database."
        )
    )
    arg.add_argument(
        '-N', '--no-update', dest='update', action='store_false',
        help=(
            "Only use the history database to compute the trend of the results"
            + " but don't store the new results in it."
        )
    )


def add_history_mandatory(arg):
    arg.add_argument(
        '-H', '--history', metavar='FILE', required=True,
        type=_ArgRealFile(existing=False),
        help=(
              "Use the history database in FILE which must be a readable"
            + " and writeable file.  If it does not exist, it will be created."
        )
    )


def add_compiler(arg):
    arg.add_argument(
        '-X', '--compiler', metavar='FILE', required=True,
        type=_ArgRealFile(existing=True),
        help=(
              "Run the benchmarks using the compiler executable FILE.  This"
            + " must be an existing executable file."
        )
    )


def add_alert(arg):
    arg.add_argument(
        '-A', '--alert', metavar='FACTOR',
        type=_ArgNumber(float, lambda x : x >= 0.0, "non-negative real number"),
        help=(
              "Alert for performance regressions greater than FACTOR sigma. "
            + " This feature is only available when a history database is"
            + " used.  In this case, the benchmark results will be compared"
            + " to the current-best result in the database and if a"
            + " regression by more than FACTOR sigma is detected, an alert"
            + " is issued and the script will report with an exit status"
            + " indicating failure.  The sigma is computed as"
            + " sqrt(s1^2 + s2^2) where s1 is the standard deviation of the"
            + " current-best result in the database and s2 is the standard"
            + " deviation of the current run.  Meaningful thresholds are"
            + " non-negative real numbers.  Values less than 1 are allowed"
            + " but probably not useful."
        )
    )


def add_constraints(arg):
    arg.add_argument(
        '-T', '--timeout', metavar='SECS',
        type=_ArgNumber(float, lambda x : x > 0.0, "positive real number"),
        help=(
            "Timeout per individual benchmark in seconds.  If a benchmark"
           + " does not prduce a result within SECS seconds, give up.  If"
           + " the benchmark did produce some result until then but the "
           + " standard deviation did not converge as desired yet, a warning"
           + " is issued and the result is used nonetheless.  Otherwise, if"
           + " there is no result at all, an error is generated."
        )
    )
    arg.add_argument(
        '-R', '--repetitions', metavar='TIMES', default=100,
        type=_ArgNumber(int, lambda x : x > 3, "integer greater than 3"),
        help=(
              "Maximum number of times to repeat a single benchmark.  If the"
            + " standard deviation did not converge as desired after TIMES"
            + " repetitions, a warning is issued and the result is used"
            + " nonetheless.  The default value if this option is not given is"
            + " to give up after %(default)d repetitions."
        )
    )
    arg.add_argument(
        '-S', '--significance', metavar='RATIO', default=0.2,
        type=_ArgNumber(float, lambda x : x > 0.0, "positive real"),
        help=(
              "Desired relative standard deviation of the results.  Benchmarks"
            + " will be run repetitively until the relative standard deviation"
            + " converges below RATIO unless a timeout occurs or the maximum"
            + " number of repetitions is exceeded earlier.  If this option is"
            + " not given, a value of %(default).2f is used."
        )
    )
    arg.add_argument(
        '-Q', '--quantile', metavar='FRACTION', default=1.0,
        type=_ArgNumber(float, lambda x : 0 < x <= 1, "real number 0 < x <= 1"),
        help=(
              "Only use the best FRACTION of the timing results.  This option"
            + " can be useful to reduce noise in the benchmark results if you"
            + " are doing other things on the computer while the benchmarks"
            + " are running.  If this option is not used, it defaults to,"
            + " %(default).2f meaning that all results will be used.  Valid"
            + " quantiles are real numbers in the interval (0, 1]."
        )
    )
    arg.add_argument(
        '-W', '--warmup', metavar='NUMBER', default=0,
        type=_ArgNumber(int, lambda x : x >= 0, "non-negative integer"),
        help=(
              "Throw away the first NUMBER of the timing results.  This option"
            + " can be useful to reduce caching effects.  If this option is"
            + " not used, it defaults to %(default)d, meaning that no results"
            + " will be thrown away."
        )
    )


def add_verbose(arg):
    arg.add_argument(
        '-V', '--verbose', action='store_true',
        help=(
              "Produce verbose logging output that tells what the script is"
            + " doing (or failing at).  This is useful for debbugging but"
            + " otherwise really annoying."
        )
    )


def add_color(arg):
    arg.add_argument(
        '-C', '--color', metavar='WHEN', nargs='?',
        type=_ArgMaybe(),
        default='auto', const='yes',
        help=(
              "Use ANSI escape sequences to produce pretty output.  This can"
            + " improve the visual experience on terminals that support them"
            + " but is strongly annoying if your terminal doesn't or if you"
            + " want to pipe the output into a file.  The values 'yes' and"
            + " 'no' unconditionally enable or disable ANSI escape sequences"
            + " respectively.  The value 'auto' will cause the script to guess"
            + " whether the output device is a terminal with support for ANSI"
            + " escape sequences and use them if so.  This is the default"
            + " behavior if this option is not used.  Specifying '--color'"
            + " without an option is equivalent to '--color=yes'."
        )
    )


def add_info(arg):
    arg.add_argument(
        '-I', '--info', action='store_true',
        help=(
              "Print additional information about how to interpret the summary"
            + " table of benchmarks results that will be printed."
        )
    )


def add_help(arg):
    arg.add_argument(
        '-?', '--help', action='help',
        help="Show this help message and exit."
    )


def use_color(when):
    if when == 'auto':
        if _os.name != 'posix':
            return False
        return all(_os.isatty(f.fileno()) for f in [_sys.stdout, _sys.stderr])
    elif when == 'yes':
        return True
    elif when == 'no':
        return False
    else:
        raise ValueError(when)


class _ArgInput(object):

    def __init__(self):
        pass

    def __call__(self, text):
        if text != '-' and not _os.path.exists(text):
            raise _argparse.ArgumentTypeError("No such file: " + text)
        else:
            return text


class _ArgRealFile(object):

    def __init__(self, existing=True):
        self.__existing = existing

    def __call__(self, text):
        if text == '-':
            raise _argparse.ArgumentTypeError("Cannot use standard input, sorry")
        elif self.__existing and not _os.path.isfile(text):
            raise _argparse.ArgumentTypeError("Not a regular file: " + text)
        else:
            return text


class _ArgDirectory(object):

    def __init__(self):
        pass

    def __call__(self, text):
        if text == '-' or not _os.path.isdir(text):
            raise _argparse.ArgumentTypeError("No such directory: " + text)
        else:
            return text


class _ArgNumber(object):

    def __init__(self, typ, check, description):
        self.__type = typ
        self.__check = check
        self.__description = description

    def __call__(self, text):
        try:
            value = self.__type(text)
            if self.__check(value):
                return value
        except ValueError:
            pass
        raise _argparse.ArgumentTypeError("Not a " + self.__description + ": " + text)


class _ArgMaybe(object):

    def __call__(self, text):
        maybe = text.lower()
        if maybe not in {'no', 'yes', 'auto'}:
            raise _argparse.ArgumentTypeError("Please say 'yes', 'no' or 'auto' but not this: " + text)
        return maybe
