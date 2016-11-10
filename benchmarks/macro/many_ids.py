#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import argparse

ap = argparse.ArgumentParser(
    description="Generate a MiniJava program with many identifiers and literals."
)
ap.add_argument('number', metavar='N', type=int, help="number of identifiers")
ns = ap.parse_args()

print("class ManyIds {")
print("")
print(
    "\tpublic int takeIt(",
    ", ".join("int p{:d}".format(i + 1) for i in range(ns.number)),
    ") {",
    sep=""
)
print(
    "\t\treturn ",
    " + ".join(["0"] + list("p{:d}".format(i + 1) for i in range(ns.number))),
    ";",
    sep=""
)
print("\t}")
print("")
print("\tpublic static void main(String[] args) {")
print("\t\tManyIds excess = new ManyIds();")
print(
    "\t\tint result = excess.takeIt(",
     ", ".join("{:d}".format(i + 1) for i in range(ns.number)),
    ");",
    sep=""
)
print("\t\tSystem.out.println(result);")
print("\t}")
print("}")
