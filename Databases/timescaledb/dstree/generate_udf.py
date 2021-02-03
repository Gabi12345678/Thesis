from datetime import datetime
from tqdm import tqdm
import argparse
import os
import numpy as np


parser = argparse.ArgumentParser(description = 'Script to run DsTree in TimescaleDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100],
	help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 30" will try (20, 30)')
parser.add_argument('--columns', nargs='*', type=int, default=[10],
	help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 30" will try (20, 30)')
parser.add_argument('--start_time', nargs='?', type=int,
	help='epoch time of the first datasample. All others will be set at 10 second intervals',
	default=1583000000)
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.implementation_path = os.path.abspath('../../../Algorithms/dstree_python/')
args.index_dir = os.path.abspath("./out")
args.index_path = os.path.abspath('./out/index')

for lines in args.lines:
	for columns in args.columns:
		print("Generating data files for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")

		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		h = open(args.file + "_index.txt", "w")
		index_lines = []
		for i in tqdm(range(lines)):
			values = f.readline()[:-1].split(" ")
			g.write(values[0] + ",\"{" + ",".join(values[1:(columns + 1)]) + "}\"\n")
			index_lines.append( values[(columns + 1) : (2 * columns + 1) ] )
		index_lines = np.array(index_lines).T.tolist()
		for i in tqdm(range(columns)):
			h.write(" ".join(index_lines[i]) + "\n")
		f.close()
		g.close()
		h.close()

		print("Generating udf")
		column_types = "d0 DOUBLE PRECISION"
		for i in range(1, columns):
			column_types = column_types + ", d" + str(i) + " DOUBLE PRECISION"
		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write( l.replace("<column_types>", column_types)
					.replace("<lines>", str(lines))
					.replace("<columns>", str(columns))
					.replace("<data_file>", args.file + "_index.txt")
					.replace("<query_file>", args.file + ".csv")
					.replace('<index_path>', args.index_path)
					.replace("<implementation_path>", args.implementation_path) )
		f.close()
		g.close()

		os.system("mkdir " + args.index_dir)
		os.system("chmod 777 " + args.index_dir)
		os.system("psql -f " + args.udf_template + ".sql")
		os.system("rm " + args.udf_template + ".sql")
		os.system("rm " + args.file + ".csv")
		os.system("rm " + args.file + "_index.txt")
		os.system("rm -r " + args.index_dir)
