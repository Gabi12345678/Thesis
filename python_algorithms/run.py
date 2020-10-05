#!/usr/bin/env python3

import argparse
import sys
import numpy as np

parser = argparse.ArgumentParser(description = 'Script to run any algorithm directly from data files.')


globalGroup = parser.add_argument_group(title = 'Global parameters')
globalGroup.add_argument('--algorithm', 
			nargs = '?', 
			help = 'Which algorithm to run.', 
			choices = ['kmeans', 'cd', 'recov', 'sax', 'hotsax', 'screen', 'znormalization', 'dstree'],
			default = 'kmeans')
globalGroup.add_argument('--datafile', 
			nargs = '?', 
			default = './data.txt',
			help = 'Path to the datafile. Values should be space separated.')
globalGroup.add_argument('--timestampfile',
				nargs = '?',
				default = './timestamps.txt',
				help = 'Path to file containing the timestamps for the data. The file should contain epoch times, one per line.')
globalGroup.add_argument('--outfile',
			nargs = '?',
			help = 'Path to the datafile where the results will be written. If missing, output to console.')


kmeansGroup = parser.add_argument_group(title = 'Kmeans parameters')
kmeansGroup.add_argument('--clusters', 
				nargs = '?', 
				type = int,
				default = 20,
				help = 'Number of clusters to generate.')
kmeansGroup.add_argument('--iterations',
				nargs = '?',
				type = int,
				default = 10,
				help = 'Number of iterations of the algorithm.')


recovGroup = parser.add_argument_group(title = 'Recov parameters')
recovGroup.add_argument('--trunc_col',
				nargs = '?',
				type = int,
				default = 1,
				help = 'Number of columns to truncate')
recovGroup.add_argument('--perc',
				nargs = '?',
				type = float,
				default = 0.1,
				help = 'Percentage of data to pre-remove, before running the algorithm')
recovGroup.add_argument('--col_drop',
				nargs = '?',
				type = int,
				default = 1,
				help = 'How many columns to pre-remove, before running the algorithm')


screenGroup = parser.add_argument_group(title = 'Screen parameters')
screenGroup.add_argument('--smax',
				nargs = '?',
				type = float,
				default = 0.01,
				help = 'Maximum speed increase for Screen')
screenGroup.add_argument('--smin',
				nargs = '?',
				type = float,
				default = -0.01,
				help = 'Minimum speed increase for Screen - Note, this is usually a negative value, to signify decrease')
screenGroup.add_argument('--windowSize',
				nargs = '?',
				type = int,
				default = 5,
				help = 'Window size for the Screen algorithm.')


dstreeGroup = parser.add_argument_group(title = 'DSTree parameters')
dstreeGroup.add_argument('--threshold',
				nargs = '?',
				type = int,
				default = 4,
				help = 'Maximum number of time series in a leaf in DSTree.')
dstreeGroup.add_argument('--indexpath',
				nargs = '?',
				default = './out/index',
				help = 'Path where to store the DSTree and it\'s timeseries')
dstreeGroup.add_argument('--buffermemory',
				nargs = '?',
				type = int,
				default = 100,
				help = 'Maximum size of the timestamps in MB that can be kept in memory when building the DSTree')
dstreeGroup.add_argument('--querypath',
				nargs = '?',
				default = './queries.txt',
				help = 'Path to the queries')
args = parser.parse_args()




print('Reading data and timestamp files')
data = np.genfromtxt(args.datafile, delimiter = ' ')
timestamps = np.genfromtxt(args.timestampfile, delimiter = ' ')

if args.algorithm == 'kmeans':
	print('Running KMeans algorithm')

	from kmeans.kmeans import kmeans
	result = kmeans(data, args.clusters, args.iterations)
	if args.outfile:
		np.savetxt(args.outfile, result, delimiter = ' ')
	
elif args.algorithm == 'cd':
	print('Running CD algorithm')

	from centroid_decomposition.cd_ssv import CD
	L, result, z = CD(data, data.shape[0], data.shape[1])
	if args.outfile:
		np.savetxt(args.outfile, result, delimiter = ' ')

elif args.algorithm == 'recov':
	print('Running Recov algorithm')

	from recovdb.recovery import recovery
	t, i, r, result = recovery(data, data.shape[0], data.shape[1], args.trunc_col, args.perc, args.col_drop)
	if args.outfile:
		np.savetxt(args.outfile, result, delimiter = ' ')

elif args.algorithm == 'sax':
	print('Running Sax algorithm')

	from sax.saxtransformation import saxrepresentation
	result = saxrepresentation(data)
	if args.outfile:
		with open(args.outfile, "w") as outfile:
			outfile.write("\n".join(result))

elif args.algorithm == 'hotsax':
	print('Running Anomaly detection(HotSax) algorithm')

	from hot_sax.hotsax import hotsax
	result = hotsax(data)
	if args.outfile:
		with open(args.outfile, "w") as outfile:
			outfile.write("\n".join( map(str, result)))

elif args.algorithm == 'screen':
	print('Running Screen algorithm')
	if len(timestamps) != len(data):
		print('Data and timestamps do not align')
		sys.exit()

	from screen.screen import screen
	result = screen(data, timestamps, args.smax, args.smin, args.windowSize)
	if args.outfile:
		np.savetxt(args.outfile, result, delimite = ' ')

elif args.algorithm == 'znormalization':
	print('Running ZNormalization algorithm')

	from znormalization.znormalization import zscore
	result = zscore(data)
	if args.outfile:
		np.savetxt(args.outfile, result, delimiter = ' ')

elif args.algorithm == 'dstree':
	print('Running DSTree algorithm')

	from dstree.dynamicsplit import IndexBuilder, IndexExactSearcher
	IndexBuilder.buildIndex(args.datafile, args.indexpath, args.threshold, 1, args.buffermemory, data.shape[0])
	queries = np.genfromtxt(args.querypath, delimiter = ' ')
	result = IndexExactSearcher.search(queries, args.indexpath + '.idx_dyn_' + str(args.threshold) + '_1_' + str(data.shape[0])) 
	if args.outfile:
		with open(args.outfile, "w") as outfile:
			outfile.write("\n".join( map(str, result) ))

if not(args.outfile):
	print(result)
