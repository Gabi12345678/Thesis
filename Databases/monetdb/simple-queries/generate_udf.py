from datetime import datetime
from tqdm import tqdm
import argparse
import os

parser = argparse.ArgumentParser(description = 'Script to run SUM in MonetDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/alabama_weather.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --column')
parser.add_argument('--column', nargs='?', type=int, default = 38,
        help='list of integers representing the number of columns to try out. Used together with --lines')
parser.add_argument('--start_time', nargs='?', type=str, default='2014-01-01', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2019-01-01', help='')
parser.add_argument('--moving_average_hours', nargs='?', type=int, default=24, help='')
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.db_dir = os.path.abspath('../master_farm')

for lines in args.lines:
	print("Generating data file for (lines) = (" + str(lines) +  ")")
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
			.replace("<db_dir>", args.db_dir)
			.replace("<start_time>", args.start_time)
			.replace("<end_time>", args.end_time)
			.replace("<moving_average_hours>", str(args.moving_average_hours)) )
	f.close()	
	g.close()
	
	os.system("mclient -p54320 -d mydb " + args.udf_template + ".sql")
	#os.system("rm " + args.udf_template + ".sql")
	os.system("rm " + args.file + ".csv")
