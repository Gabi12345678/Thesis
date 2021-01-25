from datetime import datetime
from tqdm import tqdm
import argparse
import random
import os

parser = argparse.ArgumentParser(description = 'Script to run K-Means in MonetDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/sport.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
	help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--columns', nargs='*', type=int, default = [100],
	help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=int,
	help='epoch time of the first datasample. All others will be set at 10 second intervals',
	default=1583000000)
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.implementation_path = os.path.abspath('../../../Algorithms/knn/')
args.db_dir = os.path.abspath('../master_farm')
args.output_path = os.path.abspath('output_clusters.csv')

for lines in args.lines:
	for columns in args.columns:
		print("Generating data file for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")
		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		read_lines = [""] * (2 * lines)
		for i in tqdm(range(lines * 2)):
			read_lines[i] = f.readline()
		for i in tqdm(range(lines)):
			values_a = read_lines[i][:-1].split(" ")
			values_a = values_a[:columns + 1]
			values_b = read_lines[i + lines][:-1].split(" ")[1 : columns + 1]
			g.write(",".join(values_a + values_b) + "\n")
		f.close()
		g.close()

		print("Generating udf")
		a_column_types = "a0 DOUBLE PRECISION"
		for i in range(1, columns):
			a_column_types = a_column_types + ", a" + str(i) + " DOUBLE PRECISION"
		b_column_types = a_column_types.replace("a", "b")
		distance_format = "POWER(a{0} - b{0}, 2)"
		distance_exp = distance_format.format(0)
		for i in range(1, columns):
			distance_exp = distance_exp + " + " + distance_format.format(i)
		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write( l.replace("<a_column_types>", a_column_types)
				  .replace("<b_column_types>", b_column_types)
				  .replace("<data_file>", args.file + ".csv")
				  .replace("<distance_exp>", distance_exp))
		f.close()
		g.close()

		os.system("mclient -p54320 -d mydb " + args.udf_template + ".sql")
		#os.system("rm " + args.udf_template + ".sql")
		#os.system("rm " + args.file + ".csv")
