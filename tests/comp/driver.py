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
        '--execute', metavar='FILE', type=str, const='a.out', nargs='?',
        help="execute FILE (default: '%(const)s') after successful compilation"
    )
    ap.add_argument(
        '--help', action='help',
        help="show this help message and exit"
    )
    ns = ap.parse_args(args)
    inputs = sorted(glob.glob(ns.pattern))
    if not inputs:
        raise RuntimeError("No tests found")
    return run_all(inputs, ns)


class Failure(Exception):

    pass


def run_all(inputs, ns):
    width = max(map(len, map(os.path.basename, inputs)))
    statusfmt = '{:' + str(width) + 's}   {:s}'
    failures = 0
    for src in inputs:
        displayname = os.path.basename(src)
        with open(src) as istr:
            (lines, directives) = load_program(istr)
            program = '\n'.join(lines).encode('ascii')
        try:
            if ns.escape:
                proper = program
                buggy = expose_bug(program)
                run_single_unescaped(proper, ns)
                run_single(buggy, ns, directives)
            else:
                run_single(program, ns, directives)
            print(statusfmt.format(displayname, 'PASSED'))
        except Failure as e:
            print(statusfmt.format(displayname, 'FAILED'))
            print("failure:", e, file=sys.stderr)
            failures += 1
    return failures


def expose_bug(text):
    pattern = re.compile(rb'/[*]{3}(.*?)[*]{3}/')
    return pattern.sub(rb'\1', text)


def run_single_unescaped(program, ns):
    proc = subprocess.Popen(
        ns.command,
        stdin=subprocess.PIPE,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    proc.communicate(input=program)
    if proc.returncode != 0:
        raise Failure("Compiler rejected the program even with the bug commented-out")


def run_single(program, ns, directives):
    cmd = abscmd(ns.command)
    with tempfile.TemporaryDirectory() as tempdir:
        proc = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            cwd=tempdir
        )
        (out, err) = proc.communicate(input=program)
        if proc.returncode == 0 and ns.execute is not None:
            run_executable(
                os.path.join(tempdir, ns.execute), directives, directory=tempdir)
    if proc.returncode != ns.status:
        raise Failure("Compiler exited with status {:d} instead of {:d}".format(proc.returncode, ns.status))
    if ns.message is not None:
        if not ns.message in err.decode():
            raise Failure("Compiler didn't produce the expected error message")

def run_executable(filename, directives, directory=None):
        proc = subprocess.Popen(
            filename,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            cwd=directory
        )
        (out, err) = proc.communicate()
        if proc.returncode != 0:
            raise Failure("Executable crashed with error code {:d}".format(proc.returncode))
        expected = directives.get('output')
        try:
            actual = [int(s) for s in out.split()]
        except ValueError:
            raise Failure("Executable produced output other than decimal integers")
        if expected is not None and expected != actual:
            raise Failure("Executable did not produce the expected output")

def load_program(istr):
    directive_pattern = re.compile(r'/[*]\s*[!](\w+):\s*(.*)\s*[*]/')
    lines = list()
    directives = dict()
    for line in map(lambda s : s.rstrip(), istr):
        match = directive_pattern.match(line)
        if match:
            directives[match.group(1)] = parse_directive(match.group(1), match.group(2))
        lines.append(line)
    return (lines, directives)

def parse_directive(name, value):
    if name == 'output':
        return [int(s) for s in value.split()]
    raise ValueError("Unknown directive '{}'".format(name))

def abscmd(cmd):
    copy = cmd[:]
    copy[0] = os.path.abspath(cmd[0])
    return copy

if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except (AssertionError, TypeError):
        raise
    except Exception as e:
        print("error:", e, file=sys.stderr)
        sys.exit(1)
