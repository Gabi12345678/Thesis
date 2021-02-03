from datetime import datetime
from tqdm import tqdm
import argparse
import os

parser = argparse.ArgumentParser(description = 'Script to run KNN in eXtremeDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100], help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--columns', nargs='*', type=int, default=[10], help='list of integers representing the number of lines to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
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
		for i in tqdm(range(lines * 2)):
			values = f.readline()[:-1].split(" ")
			values = values[:columns + 1]
			g.write(",".join(values) + "\n")
		f.close()
		g.close()

		print("Generating udf")
		column_types = 'd0 DOUBLE'
		for i in range(1, columns):
			column_types = column_types + ', d' + str(i) + ' DOUBLE'
		column_values = "d0"
		for i in range(1, columns):
			column_values = column_values + ", d" + str(i)

		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write(l.replace("<column_types>", column_types)
				.replace("<column_values>", column_values)
				.replace("<db_dir>", args.db_dir)
				.replace("<implementation_path>", args.implementation_path)
				.replace("<start_time>", str(args.start_time))
				.replace("<lines>", str(lines))
				.replace("<columns>", str(columns))
				.replace("<input_file>", args.file + ".csv"))
		f.close()
		g.close()

		os.system("rm mydb*")
		os.system("MCO_PYTHONAPILIB=libmcopythonapi.so LD_LIBRARY_PATH=../eXtremeDB/target/bin.so/ ../eXtremeDB/target/bin/xsql -c db.config -f udf_template.sql.sql -b")
		os.system("rm " + args.file + ".csv")
os.system("rm " + args.udf_template + ".sql")
os.system("rm mydb*")
