#!/bin/bash

gnuplot <<- EOF
	set term png
	set output 'data.png'
	set xlabel '$LabelX'
	set ylabel '$LabelY'
	set grid
	set key above
	set xrange [$minX : $maxX]
	set yrange [$minY : $maxY]
	plot 'graph_data.dat'
EOF

display 'data.png'
#clear
