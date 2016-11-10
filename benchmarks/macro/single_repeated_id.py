#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse
import random
import sys

ap = argparse.ArgumentParser(
    description="Generate lexically correct input with the same identifier repeated."
)
ap.add_argument(
    'number', metavar='N', type=int,
    help="number of repetitions"
)
ap.add_argument(
    '--length', metavar='M', type=int, default=12,
    help="minimum length of the identifier"
)
ns = ap.parse_args()

if ns.number < 0:
    print("Please select N >= 0 or it's not gonna work", file=sys.stderr)
    sys.exit(1)

if ns.length < 1:
    print("Please select M >= 1 or it's not gonna work", file=sys.stderr)
    sys.exit(1)

idhead = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_'
idtail = idhead + '0123456789'

symbol = random.choice(idhead) + ''.join(random.choice(idtail) for j in range(ns.length - 1))

for i in range(ns.number):
    print(symbol)
