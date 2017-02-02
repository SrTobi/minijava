#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse
import itertools
import os
import shutil
import sys


PROGRAM='blobdump'

SPECIAL_ASCII_C = {
    '\0' : r'\0',
    '\a' : r'\a',
    '\b' : r'\b',
    '\t' : r'\t',
    '\n' : r'\n',
    '\v' : r'\v',
    '\f' : r'\f',
    '\r' : r'\r',
    '"'  : r'\"',
    '?'  : r'\?',
    '\\' : r'\\',
}


def main(args):
    ap = argparse.ArgumentParser(
        prog=PROGRAM,
        usage="%(prog)s (-t | -b) [-o FILE] FILE...",
        description="Dump arbitrary files as C source code.",
    )
    ap.add_argument(
        '-t', '--text', action='store_const', dest='mode', const='T',
        help="interpret input as ASCII text and output one escaped string per line"
    )
    ap.add_argument(
        '-b', '--binary', action='store_const', dest='mode', const='B',
        help="interpret input as binary data and output one comma-separated integer per byte"
    )
    ap.add_argument(
        '-o', '--output', metavar='FILE', type=str,
        help="output to FILE instead of writing to standard output"
    )
    ap.add_argument(
        'inputs', metavar='FILE', type=str, nargs='*',
        help="process FILEs (one after another) instead of reading form standard input"
    )
    ns = do_parse_args(ap, args)
    if ns.mode is None:
        raise RuntimeError("Please specify a mode ('--text' or '--binary')")
    (process, smart_open_in) = {
        'B' : (process_binary, smart_open_in_binary),
        'T' : (process_text,   smart_open_in_text),
    }[ns.mode]
    with smart_open_out(ns.output) as ostr:
        filenames = ns.inputs if ns.inputs else [None]
        for filename in filenames:
            with smart_open_in(filename) as istr:
                process(istr, ostr, filename=filename)


def do_parse_args(ap, args):
    termsize = shutil.get_terminal_size()
    try:
        os.environ['LINES'] = str(termsize.lines)
        os.environ['COLUMNS'] = str(termsize.columns)
        return ap.parse_args(args)
    finally:
        del os.environ['LINES']
        del os.environ['COLUMNS']


def process_binary(istr, ostr, filename=None):
    data = istr.read()
    if filename is not None:
        print("/* {} ({} bytes) */".format(filename, len(data)), file=ostr)
    for chunk in grouped(data, 16):
        print(
            ', '.join(map(lambda b : '0x{:02x}'.format(b), chunk)),
            end=',\n', file=ostr
        )

def process_text(istr, ostr, filename=None):
    lines = istr.readlines()
    if filename is not None:
        print("/* {} ({} lines) */".format(filename, len(lines)), file=ostr)
    for line in lines:
        print('"' + ''.join(map(escape_ascii_c, line)) + '"', file=ostr)


def escape_ascii_c(c):
    if c in SPECIAL_ASCII_C:
        return SPECIAL_ASCII_C[c]
    elif ord(' ') <= ord(c) <= ord('~'):
        return c
    else:
        return r'\x{:02x}'.format(ord(c))


def smart_open_in_text(filename=None):
    if filename is None or filename == '-':
        return sys.stdin
    return open(filename, 'rt')


def smart_open_in_binary(filename=None):
    if filename is None or filename == '-':
        return sys.stdin.buffer
    return open(filename, 'rb')


def smart_open_out(filename=None):
    if filename is None or filename == '-':
        return sys.stdout
    return open(filename, 'w')


def grouped(iterable, n):
    args = [iter(iterable)] * n
    return map(
        lambda seq : filter(lambda x : x is not None, seq),
        itertools.zip_longest(*args)
    )


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1 : ]))
    except (AssertionError, TypeError):
        raise
    except Exception as e:
        print("{}: error: {}".format(PROGRAM, str(e)), file=sys.stderr)
        sys.exit(1)
