#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

from email.utils import formatdate
import argparse
import itertools
import random
import sys
import time


PROGRAM = 'keyword-hash.py'

TIMESTAMP = formatdate()

KEYWORDS = frozenset("""
 abstract        assert          boolean         break           byte
 case            catch           char            class           const
 continue        default         double          do              else
 enum            extends         false           finally         final
 float           for             goto            if              implements
 import          instanceof      interface       int             long
 native          new             null            package         private
 protected       public          return          short           static
 strictfp        super           switch          synchronized    this
 throws          throw           transient       true            try
 void            volatile        while
""".split())

MIN_KEYWORD_LENGTH = min(map(len, KEYWORDS))

MAX_KEYWORD_LENGTH = max(map(len, KEYWORDS))


class PerfectResult(object):

    def __init__(self, pearson, hashes, iterations, elapsed):
        self.pearson = pearson
        self.hashes = hashes
        self.iterations = iterations
        self.elapsed = elapsed


def main():
    ap = argparse.ArgumentParser(
        prog=PROGRAM,
        description=(
            "Generates C++ code for lookup tables for a perfect 8 bit Pearson"
            " hash function for MiniJava keywords."
        )
    )
    ap.add_argument(
        '-s', '--seed', metavar='NUMBER', type=(lambda s : int(s, 0)),
        help="use explicit random seed NUMBER insted of non-determinism"
    )
    ap.add_argument(
        '-o', '--output', metavar='FILE', type=str,
        help="write generated C++ code to FILE instead of standard output"
    )
    ns = ap.parse_args()
    seed = ns.seed
    if seed is None:
        rnddev = random.SystemRandom()
        seed = rnddev.getrandbits(128)
    pr = find_perfect(seed)
    with smart_open(ns.output) as sys.stdout:
        print_cxx(pr, seed)

def find_perfect(seed=None):
    t0 = time.time()
    rng = random.Random(seed)
    pearson = list(range(256))
    i = 0
    while True:
        i += 1
        hashes = {hash_word(w, pearson) : w for w in KEYWORDS}
        if len(hashes) == len(KEYWORDS):
            break
        rng.shuffle(pearson)
    t1 = time.time()
    return PerfectResult(pearson, hashes, i, t1 - t0)

def hash_word(word, pearson):
    h = 0
    for c in map(ord, word):
        h ^= pearson[c]
    return h

def print_cxx(pr, seed=None):
    level = 0
    print_header_comment(pr, seed)
    out()
    print_include_guard(
        'MINIJAVA_INCLUDED_FROM_LEXER_KEYWORD_HPP',
        'lexer/keyword_pearson.tpp',
        'lexer/keyword.hpp'
    )
    out()
    print_ascii_static_assert()
    out()
    for hdr in ['cstddef', 'cstdint']:
        out(level, '#include <{}>'.format(hdr))
    out()
    for hdr in ['lexer/token_type.hpp']:
        out(level, '#include "{}"'.format(hdr))
    out()
    for ns in ['minijava', 'detail']:
        out(level, 'namespace {}'.format(ns))
        out(level, '{')
        level += 1
    out()
    out(level, 'constexpr std::size_t keyword_min_length = {};'.format(MIN_KEYWORD_LENGTH))
    out()
    out(level, 'constexpr std::size_t keyword_max_length = {};'.format(MAX_KEYWORD_LENGTH))
    out()
    out(level, 'constexpr std::uint8_t keyword_pearson_table[256] = {')
    for chunk in grouped(pr.pearson, 16):
        out(level + 1, ', '.join(map(lambda x : '0x{:02x}'.format(x), chunk)) + ',')
    out(level, '};')
    out()
    out(level, 'constexpr token_type keyword_lookup_table[256] = {')
    for i in range(256):
        out(level + 1, format_lut_entry(i, pr.hashes) + ',')
    out(level, '};')
    out()
    while level > 0:
        out(level - 1, '}')
        level -= 1

def format_lut_entry(i, hashes):
    prefix = 'token_type::'
    if i in hashes:
        return prefix + 'kw_' + hashes[i]
    else:
        return prefix + 'identifier'

def print_ascii_static_assert():
    characters = sorted(set(''.join(KEYWORDS)))
    format_one = lambda c : "('{:s}' == 0x{:02x})".format(c, ord(c))
    out(0, "// We only assert on characters that actually appear in keywords.")
    out(0, "static_assert(")
    for (i, chunk) in enumerate(grouped(characters, 4)):
        chunk = list(filter(lambda x : x is not None, chunk))
        prefix = '   ' if i == 0 else '&& '
        postfix = ',' if 4 * i + len(chunk) == len(characters) else ''
        out(1, prefix + ' && '.join(map(format_one, chunk)) + postfix)
    out(1, '"Non-ASCII character set not supported"')
    out(0, ');')

def print_include_guard(guard, notthis, butthat):
    out(0, '#ifndef {}'.format(guard))
    out(0, '#error "Never `#include <{}>` directly; `#include <{}>` instead."'.format(notthis, butthat))
    out(0, '#endif')

def print_header_comment(pr, seed=None):
    outhdr()
    outhdr("")
    outhdr("THIS IS A GENERATED FILE, DO NOT EDIT!")
    outhdr("")
    outhdr("Generated by '{}' on {}".format(PROGRAM, TIMESTAMP))
    outhdr("Perfect solution found after {:.3f} seconds in {:d} iterations".format(
        pr.elapsed, pr.iterations)
    )
    if seed is not None:
        outhdr("Run with '--seed=0x{:x}' to reproduce".format(seed))
    outhdr("")
    outhdr()

def outhdr(text=None):
    if text is None:
        text = '-' * 70
    out(0, '/* {:^70s} */'.format(text))

def out(level=0, text=None):
    assert type(level) is int
    if text is None:
        print()
    else:
        print('\t' * level, text, sep='')

def smart_open(filename=None):
    if filename is None or filename == '-':
        return sys.stdout
    return open(filename, 'w')

def grouped(iterable, n):
    args = [iter(iterable)] * n
    return itertools.zip_longest(*args)


if __name__ == '__main__':
    main()
