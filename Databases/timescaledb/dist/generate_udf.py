from datetime import datetime
from tqdm import tqdm
import argparse
import os
import random


parser = argparse.ArgumentParser(description = 'Script to run K-means in TimescaleDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/sport.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100],
	help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 30" will try (20, 30)')
parser.add_argument('--columns', nargs='*', type=int, default=[100],
	help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 30" will try (20, 30)')
parser.add_argument('--start_time', nargs='?', type=int,
	help='epoch time of the first datasample. All others will be set at 10 second intervals',
	default=1583000000)
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.implementation_path = os.path.abspath('../../../Algorithms/knn/')

for lines in args.lines:
	for columns in args.columns:
		print("Generating data file for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")
		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		read_lines = []
		for i in tqdm(range(lines * 2)):
			read_lines.append( f.readline() )
		for i in tqdm(range(lines)):
			values_a = read_lines[i][:-1].split(" ")
			values_b = read_lines[i + lines][:-1].split(" ")
			to_write = values_a[0]
			to_write = to_write + ",\"{" + ",".join(values_a[1:columns + 1]) + "}\""
			to_write = to_write + ",\"{" + ",".join(values_b[1:columns + 1]) + "}\"\n"
			g.write(to_write)
		f.close()
		g.close()

		print("Generating udf")
		distance_format = "power(a[{0}]-b[{0}],2.0)"
		distance_exp = distance_format.format(1)
		for i in range(1, columns):
			distance_exp = distance_exp + " + " + distance_format.format(i + 1)
		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write( l
				.replace("<data_file>", args.file + ".csv")
				.replace("<distance_exp>", distance_exp) )
		f.close()
		g.close()
		
		os.system("psql -f " + args.udf_template + ".sql")
		#os.system("rm " + args.udf_template + ".sql")
		#os.system("rm " + args.file + ".csv")
