#!/bin/bash
rm results -r
mkdir results

# CentroidDecomposition
./run.py --algorithm cd --datafile ./data.txt --outfile ./results/cd.txt

# HOTSAX
./run.py --algorithm hotsax --datafile ./data.txt --saxwindow 40 --outfile ./results/hotsax.txt

# KMeans
./run.py --algorithm kmeans --datafile ./data.txt --clusters 10 --iterations 20 --outfile ./results/kmeans.txt

# Recov
./run.py --algorithm recov --datafile ./data.txt --trunc_col 45 --perc 0.1 --col_drop 2 --outfile ./results/recov.txt

# SAX
./run.py --algorithm sax --datafile ./data.txt --outfile ./results/sax.txt

# Screen
./run.py --algorithm screen --datafile ./data.txt --timestampfile ./timestamps.txt --smax 0.01 --smin -0.01 --screenwindow 5 --outfile ./results/screen.txt

# ZNormalization
./run.py --algorithm znormalization --datafile ./data.txt --outfile ./results/znorm.txt

# DSTree
rm tmp -r
mkdir tmp
./run.py --algorithm dstree --datafile ./data_transpose.txt --threshold 5 --indexpath ./tmp/index --buffermemory 100 --querypath ./queries.txt --outfile ./results/dstree.txt
