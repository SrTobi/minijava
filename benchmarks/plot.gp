#! /usr/bin/gnuplot

set terminal svg size 800,600
set output 'benchmark.svg'

set xdata time
set timefmt '%s'

set xlabel "point in time"
set ylabel "benchmark result / s"

set xrange [* : *]
set yrange [0 : *]

set nokey

plot 'benchmark.dat' using 1:2:3 with yerror
