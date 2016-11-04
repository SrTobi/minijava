#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse

ap = argparse.ArgumentParser(
    description=(
          "Generate file with a large number of /* */ comments followed"
        + " by a single integer literal."
    )
)
ap.add_argument(
    'number', metavar='N', type=int,
    help="number of consecutive block-comments to generate"
)
ns = ap.parse_args()

for i in range(ns.number):
    print('/* A blok-comment on line {:d} */'.format(i + 1))
print("42")
