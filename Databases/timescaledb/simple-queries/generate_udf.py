from datetime import datetime
from tqdm import tqdm
import argparse
import os


parser = argparse.ArgumentParser(description = 'Script to run Simple queries in TimescaleDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/alabama_weather.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100],
	help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 1" will try (20, 1)')
parser.add_argument('--column', nargs='?', type=int, default=38,
	help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 1" will try (20, 1)')
parser.add_argument('--start_time', nargs='?', type=str, default='2014-01-01', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2019-01-01', help='')
parser.add_argument('--moving_average_hours', nargs='?', type=int, default=24, help='')
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')

for lines in args.lines:
	print("Generating data file for (lines, columns) = (" + str(lines) + ", " + str(args.column) + ")")
	f = open(args.file, "r")
	g = open(args.file + ".csv", "w")
	for i in tqdm(range(lines)):
		values = f.readline()[:-1].split(" ")
		g.write(values[0] + ',' + values[args.column + 1] + '\n')
	f.close()
	g.close()

	print("Generating udf")
	f = open(args.udf_template, "r")
	g = open(args.udf_template + ".sql", "w")
	for l in f.readlines():
		g.write( l.replace("<lines>", str(lines))
				.replace("<data_file>", args.file + ".csv")
				.replace("<start_time>", args.start_time)
				.replace("<end_time>", args.end_time)
				.replace("<moving_average_hours>", str(args.moving_average_hours)) )
	f.close()
	g.close()
		
	os.system("psql -f " + args.udf_template + ".sql")
	#os.system("rm " + args.udf_template + ".sql")
	os.system("rm " + args.file + ".csv")
