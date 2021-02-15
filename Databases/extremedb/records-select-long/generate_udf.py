from datetime import datetime
from tqdm import tqdm
import argparse
import os

parser = argparse.ArgumentParser(description = 'Script to run K-Means in eXtremeDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100], help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--columns', nargs='*', type=int, default=[10], help='list of integers representing the number of lines to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=str, default='2021-01-31', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2021-02-01', help='')
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.implementation_path = os.path.abspath('../../../Algorithms/kmeans')
args.db_dir = os.path.abspath('./')

for lines in args.lines:
	for columns in args.columns:
		print("Generating data file for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")
		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		for i in tqdm(range(lines)):
			values = f.readline()[:-1].split(" ")
			values = values[:columns + 1]
			g.write(",".join(values) + "\n")
		f.close()
		g.close()

		moving_average_format = "AVG(d[{0}]) OVER (ORDER BY t ASC RANGE BETWEEN 1440 PRECEDING AND 0 FOLLOWING)"
		moving_average_formula = ",".join( [moving_average_format.format(x) for x in range(columns)] )
		print("Generating udf")
		f = open(args.udf_template, "r")
		g = open(args.udf_template + ".sql", "w")
		for l in f.readlines():
			g.write(l
				.replace("<moving_average_formula>", moving_average_formula)
				.replace("<start_time>", args.start_time)
				.replace("<end_time>", args.end_time)
				.replace("<lines>", str(lines))
				.replace("<columns>", str(columns))
				.replace("<input_file>", args.file + ".csv"))
		f.close()
		g.close()

		os.system("rm mydb*")
		os.system("MCO_PYTHONAPILIB=libmcopythonapi.so LD_LIBRARY_PATH=../eXtremeDB/target/bin.so/ ../eXtremeDB/target/bin/xsql -c db.config -f udf_template.sql.sql -b")
		os.system("rm " + args.file + ".csv")
#os.system("rm " + args.udf_template + ".sql")
os.system("rm mydb*")
