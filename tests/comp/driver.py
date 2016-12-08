#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse
import glob
import os.path
import re
import subprocess
import sys
import tempfile


def main(args):
    ap = argparse.ArgumentParser(
        add_help=False,
        usage="%(prog)s --input=PATTERN [OPTION...] [--] COMMAND ...",
        description=(
              "Successively run the compiler on multiple input files. "
            + " For each file that is matches by the specified pattern,"
            + " the compiler command-line is executed and the file fed to it"
            + " as input.  Output is discarded.  If the exit code of the"
            + " compiler does not match the expected status or if the expected"
            + " error message was not produces, the test is considered a"
            + " failure.  Otherwise, it is considered a success."
        )
    )
    ap.add_argument(
        'command', metavar='COMMAND', nargs='+',
        help="command-line to invoke the compiler"
    )
    ap.add_argument(
        '--input', metavar='PATTERN', dest='pattern', type=str, required=True,
        help="POSIX globbing expression to select input files"
    )
    ap.add_argument(
        '--status', metavar='N', default=0, type=int,
        help="expected exit status of the compiler (default: %(default)d)"
    )
    ap.add_argument(
        '--message', metavar='STRING', default=None, type=str,
        help="expected error message"
    )
    ap.add_argument(
        '--escape', action='store_true',
        help="expect success unless '/*** ***/ comments are uncommented"
    )
    ap.add_argument(
        '--help', action='help',
        help="show this help message and exit"
    )
    ns = ap.parse_args(args)
    inputs = sorted(glob.glob(ns.pattern))
    return run_all(inputs, ns.command, status=ns.status, message=ns.message, escape=ns.escape)


class Failure(Exception):

    pass


def run_all(inputs, command, status=None, message=None, escape=None):
    width = max(map(len, map(os.path.basename, inputs)))
    statusfmt = '{:' + str(width) + 's}   {:s}'
    failures = 0
    for src in inputs:
        displayname = os.path.basename(src)
        with open(src, 'rb') as istr:
            program = istr.read()
        try:
            if escape:
                proper = program
                buggy = expose_bug(program)
                run_single_unescaped(command, proper)
                run_single(command, buggy, status=status, message=message)
            else:
                run_single(command, program, status=status, message=message)
            print(statusfmt.format(displayname, 'PASSED'))
        except Failure as e:
            print(statusfmt.format(displayname, 'FAILED'))
            print("failure:", e, file=sys.stderr)
            failures += 1
    return failures


def expose_bug(text):
    pattern = re.compile(rb'/[*]{3}(.*?)[*]{3}/')
    return pattern.sub(rb'\1', text)


def run_single_unescaped(cmd, program):
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    proc.communicate(input=program)
    if proc.returncode != 0:
        raise Failure("Compiler rejected the program even with the bug commented-out")


def run_single(cmd, program, status=None, message=None):
    abscmd = cmd[:]
    abscmd[0] = os.path.abspath(cmd[0])
    with tempfile.TemporaryDirectory() as tempdir:
        proc = subprocess.Popen(
            abscmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            cwd=tempdir
        )
        (out, err) = proc.communicate(input=program)
    if proc.returncode != status:
        raise Failure("Compiler exited with status {:d} instead of {:d}".format(proc.returncode, status))
    if message is not None:
        if not message in err.decode():
            raise Failure("Compiler didn't produce the expected error message")


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except (AssertionError, TypeError):
        raise
    except Exception as e:
        print("error:", e, file=sys.stderr)
        raise
        sys.exit(1)
