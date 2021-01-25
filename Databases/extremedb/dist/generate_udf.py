from datetime import datetime
from tqdm import tqdm
import argparse
import os
import random

parser = argparse.ArgumentParser(description = 'Script to run KNN in eXtremeDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/sport.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[1000], help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--columns', nargs='*', type=int, default=[100], help='list of integers representing the number of lines to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=int, help='epoch time of the first datasample. All other will be set at 10 seconds intervals', default=1583000000)
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.implementation_path = os.path.abspath('../../../Algorithms/knn')
args.db_dir = os.path.abspath('./')

for lines in args.lines:
	for columns in args.columns:
		print("Generating data file for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")
		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		read_lines = []
		for i in tqdm(range(2 * lines)):
			read_lines.append( f.readline() )
		for i in tqdm(range(lines)):
			values_a = read_lines[i][:-1].split(" ")
			values_a = values_a[:columns + 1]
			values_b = read_lines[i + lines][:-1].split(" ")
			values_b = values_b[1 : (columns+1)]
			g.write(",".join(values_a + values_b) + "\n")
		f.close()
		g.close()

		print("Generating udf")
		distance_format = "(a[{0}] - b[{0}])*(a[{0}] - b[{0}])"
		distance_formula = distance_format.format(0)
		for i in range(1, columns):
			distance_formula = distance_formula + " + " +  distance_format.format(i)

		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write(l.replace("<db_dir>", args.db_dir)
				.replace("<implementation_path>", args.implementation_path)
				.replace("<start_time>", str(args.start_time))
				.replace("<lines>", str(lines))
				.replace("<columns>", str(columns))
				.replace("<data_file>", args.file + ".csv")
				.replace("<distance_formula>", distance_formula))
		f.close()
		g.close()

		os.system("rm mydb*")
		os.system("PYTHONPATH=../eXtremeDB/target/bin/python ../eXtremeDB/target/bin/xsql -c db.config -f udf_template.sql.sql -b")
		os.system("rm " + args.file + ".csv")
#os.system("rm " + args.udf_template + ".sql")
os.system("rm mydb*")
