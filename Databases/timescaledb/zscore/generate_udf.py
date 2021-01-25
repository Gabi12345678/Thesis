from datetime import datetime
from tqdm import tqdm
import argparse
import os


parser = argparse.ArgumentParser(description = 'Script to run Z-Score in TimescaleDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/hydraulic.txt')
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
args.implementation_path = os.path.abspath('../../../Algorithms/znormalization/')

for lines in args.lines:
	for columns in args.columns:
		print("Generating data file for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")
		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		for i in tqdm(range(lines)):
			values = f.readline()[:-1].split(" ")
			g.write(values[0] + ",\"{" + ",".join(values[1:(columns + 1)]) + "}\"\n")
		f.close()
		g.close()

		print("Generating udf")
		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write( l.replace("<lines>", str(lines)).replace("<columns>", str(columns)).replace("<data_file>", args.file + ".csv").replace("<implementation_path>", args.implementation_path) )
		f.close()
		g.close()

		os.system("psql -f " + args.udf_template + ".sql")
		os.system("rm " + args.udf_template + ".sql")
		os.system("rm " + args.file + ".csv")
