#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse
import glob
import os.path
import re
import shutil
import subprocess
import sys
import tempfile


def main(args):
    ap = argparse.ArgumentParser(
        add_help=False,
        usage="%(prog)s --input=PATTERN [OPTION...] [--] COMMAND ...",
        description=(
              "Successively run the compiler on multiple input files. "
            + " For each file that is matched by the specified pattern,"
            + " the compiler command-line is executed and the file fed to it"
            + " as input.  Various command line options and directives inside"
            + " the file control what checks are performed in order for the"
            + " test to be considered a success or failure."
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
        '--define', metavar='SYMBOLS', type=ArgPPSymbolList(),
        help="comma-separated list of pre-processor symbols to pre-define"
    )
    ap.add_argument(
        '--execute', metavar='FILE', type=str, const='a.out', nargs='?',
        help="execute FILE (default: '%(const)s') after successful compilation"
    )
    ap.add_argument(
        '--debug', action='store_true',
        help="dump internal state for debugging the test driver"
    )
    ap.add_argument(
        '--help', action='help',
        help="show this help message and exit"
    )
    ns = do_parse_args(ap, args)
    inputs = sorted(glob.glob(ns.pattern))
    ppsymbols = frozenset(ns.define) if ns.define else frozenset()
    if ns.define is not None and len(ppsymbols) != len(ns.define):
        raise SetupError("Duplicate pre-defined symbols")
    if ns.debug:
        print("DEBUG: Parsed command-line arguments:")
        print("DEBUG:")
        for (k, v) in vars(ns).items():
            print("DEBUG: {:>12s}:  {:s}".format(k, repr(v)))
        print("DEBUG:")
        print("DEBUG: Pre-defined pre-processor symbols:", repr(sorted(ppsymbols)))
        print("DEBUG: Matched input files:", inputs)
    if not inputs:
        raise SetupError("No tests found")
    return run_all(inputs, ppsymbols, ns)


def do_parse_args(ap, args):
    termsize = shutil.get_terminal_size()
    os.environ['LINES'] = str(termsize.lines)
    os.environ['COLUMNS'] = str(termsize.columns)
    ns = ap.parse_args(args)
    del os.environ['LINES']
    del os.environ['COLUMNS']
    return ns


class ArgPPSymbolList(object):

    def __call__(self, text):
        pattern = re.compile(r'[A-Z][A-Z0-9_]*')
        tokens = list(map(str.strip, text.split(',')))
        for token in tokens:
            if not pattern.match(token):
                raise argparse.ArgumentTypeError("Not a valid preprocessing symbol: " + token)
        return tokens


class Failure(Exception):

    pass


class SetupError(Exception):

    pass


def run_all(inputs, ppsymbols, ns):
    width = max(map(len, map(os.path.basename, inputs)))
    statusfmt = '{:' + str(width) + 's}   {:s}'
    failures = 0
    for src in inputs:
        displayname = os.path.basename(src)
        if ns.debug:
            print("DEBUG: Running test '{:s}' ...".format(src))
        with open(src) as istr:
            (program, pragmas) = load_program(istr, ppsymbols, ns.debug)
        if 'skip' in pragmas:
            print(statusfmt.format(displayname, 'SKIPPED'))
            continue
        try:
            run_single(program, ns, pragmas)
            print(statusfmt.format(displayname, 'PASSED'))
        except Failure as e:
            print(statusfmt.format(displayname, 'FAILED'))
            if 'failure' not in pragmas:
                failures += 1
                print("failure:", e, file=sys.stderr)
            else:
                print("failure (known issue):", e, file=sys.stderr)
        if ns.debug:
            print("DEBUG:")
    return failures


def run_single(program, ns, pragmas):
    with tempfile.TemporaryDirectory() as tempdir:
        if ns.debug:
            print("DEBUG: Using temporary directory '{:s}'".format(tempdir))
        run_compiler(program, ns, pragmas, tempdir)
        if ns.execute:
            run_executable(ns, pragmas, tempdir)


def run_compiler(program, ns, pragmas, directory=None):
    cmd = abscmd(ns.command)
    if ns.debug:
        print("DEBUG: Running compiler as {:s} ...".format(repr(cmd)))
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=directory
    )
    (out, err) = proc.communicate(input=program.encode('ascii'))
    if ns.debug:
        log_popen_result(proc, out, err)
    expected = pragmas.get('status', 0)
    if proc.returncode != expected:
        raise Failure("Compiler exited with status {:d} instead of {:d}".format(proc.returncode, expected))
    if 'message' in pragmas:
        outer_pattern = re.compile(r'error: (.*)\n')
        match = outer_pattern.match(err.decode())
        if not match:
            raise Failure("Compiler did not produce any error message")
        inner_pattern = pragmas['message']
        if inner_pattern is not None and not inner_pattern.match(match.group(1)):
            raise Failure("Compiler didn't produce the expected error message")


def run_executable(ns, pragmas, directory=None):
    assert ns.execute
    executable = ns.execute if directory is None else os.path.join(directory, ns.execute)
    if ns.debug:
        print("DEBUG: Running executable '{:s}' ...".format(executable))
    proc = subprocess.Popen(
        [executable],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=directory
    )
    (out, err) = proc.communicate()
    if ns.debug:
        log_popen_result(proc, out, err)
    if proc.returncode != 0:
        raise Failure("Executable crashed with error code {:d}".format(proc.returncode))
    try:
        actual = [int(s) for s in out.split()]
    except ValueError:
        raise Failure("Executable produced output other than decimal integers")
    expected = pragmas.get('output')
    if expected is not None and expected != actual:
        raise Failure("Executable did not produce the expected output")


def log_popen_result(proc, out, err):
    print("DEBUG: Child process with PID {:d} quit with exit status {:d}".format(
        proc.pid, proc.returncode
    ))
    log_popen_stream(out, "output")
    log_popen_stream(err, "error")


def log_popen_stream(data, what):
    if not data:
        print("DEBUG: Standard {:s} was empty".format(what))
        return
    try:
        lines = data.decode().splitlines()
        print("DEBUG: Standard {:s} was:".format(what))
        print("DEBUG:")
        for (i, line) in enumerate(lines):
            print("DEBUG: {:5d}:   {:s}".format(1 + i, line))
        print("DEBUG:")
    except UnicodeDecodeError:
        print("DEBUG: Standard {:s} was {:d} bytes of non-textual data".format(what, len(data)))


def load_program(istr, ppsymbols, debug=False):
    lines = list()
    pragmas = dict()
    collect = [True]
    for line in map(str.rstrip, istr):
        stripline = line.lstrip()
        if stripline.startswith('//'):
            lines.append("")
            (doubleslash, *tokens) = stripline.split()
            if doubleslash != '//':
                raise SetupError("'//' Comments must be followed by white space")
            if not tokens:
                pass
            elif tokens[0] == 'ifdef':
                pp_check_arity(1, tokens)
                collect.append(tokens[1] in ppsymbols)
            elif tokens[0] == 'ifndef':
                pp_check_arity(1, tokens)
                collect.append(tokens[1] not in ppsymbols)
            elif tokens[0] == 'else':
                pp_check_arity(0, tokens)
                collect.append(not collect.pop())
            elif tokens[0] == 'endif':
                pp_check_arity(0, tokens)
                collect.pop()
                if not collect:
                    raise SetupError("Pragma 'endif' without matching 'if'")
            elif tokens[0] == 'pragma':
                (_, pragma, *args) = tokens
                value = pp_parse_pragma(pragma, args)
                if all(collect):
                    if pragma in pragmas:
                        raise SetupError("Pragma '{:s}' cannot be used more than once".format(pragma))
                    pragmas[pragma] = value
            else:
                raise SetupError("Unknown pre-processing directive: {:s}".format(tokens[0]))
        elif all(collect):
            lines.append(line)
        else:
            lines.append("")
    if len(collect) != 1:
        raise SetupError("File ended before 'if' directive was closed by 'endif'")
    if debug:
        log_loaded_test(lines, pragmas)
    return ('\n'.join(lines) + '\n', pragmas)


def log_loaded_test(lines, pragmas):
    print("DEBUG: Pre-processed program text:")
    print("DEBUG:")
    for (i, line) in enumerate(lines):
        print("DEBUG: {:5d}:   {:s}".format(1 + i, line))
    print("DEBUG:")
    print("DEBUG: Active pragmas:")
    print("DEBUG:")
    for (pragma, value) in pragmas.items():
        print("DEBUG: {:>12s}:  {:s}".format(pragma, repr(value)))
    print("DEBUG:")


def pp_check_arity(expected, tokens):
    assert type(expected) is int
    assert type(tokens) is list
    assert all(map(lambda x : type(x) is str, tokens))
    assert tokens
    actual = len(tokens) - 1
    if expected != actual:
        # TRANSLATORS: Oh, never mind...
        w1 = 'argument' if expected == 1 else 'arguments'
        w2 = 'was' if actual == 1 else 'were'
        raise SetupError((
            "Pre-processing directive '{:s}' takes {:d} {:s} but {:d} {:s} given"
        ).format(tokens[0], expected, w1, actual, w2))


def pp_parse_pragma(name, args):
    assert type(name) is str
    assert type(args) is list
    assert all(map(lambda x : type(x) is str, args))
    if name in {'skip', 'failure'}:
        if len(args) != 0:
            raise SetupError("Pragma '{:s}' must not be given any arguments".format(name))
        return None
    elif name == 'message':
        if not args:
            return None
        elif len(args) == 1 and len(args[0]) >= 2:
            if args[0].startswith('"') and args[0].endswith('"'):
                (_, *value, _) = args[0]
                return re.compile(re.escape(value))
            if args[0].startswith('/') and args[0].endswith('/'):
                (_, *value, _) = args[0]
                try:
                    return re.compile(value)
                except re.error as e:
                    raise SetupError("Invalid regex: " + str(e))
        raise SetupError("Pragma 'message' may only be followed by a quoted string or regex")
    elif name == 'output':
        try:
            return list(map(int, args))
        except ValueError:
            raise SetupError("Pragma 'output' must be followed by a list of integers")
    elif name == 'status':
        if len(args) == 1:
            try:
                return int(args[0])
            except ValueError:
                pass
        raise SetupError("Pragma 'status' must be followed by exactly one integer")
    else:
        raise SetupError("Unknown directive '{:s}'".format(name))


def abscmd(cmd):
    (head, *tail) = cmd
    return [os.path.abspath(cmd[0]), *tail]


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except (AssertionError, TypeError, NameError):
        raise
    except Exception as e:
        print("error:", e, file=sys.stderr)
        sys.exit(1)
