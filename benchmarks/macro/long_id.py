#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse

ap = argparse.ArgumentParser(
    description=("Generate file with a single long identifier.")
)
ap.add_argument(
    'length', metavar='N', type=int,
    help="length of the identifier"
)
ns = ap.parse_args()

print('a' * ns.length)
