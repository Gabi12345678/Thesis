from datetime import datetime
from tqdm import tqdm
import argparse
import os

parser = argparse.ArgumentParser(description = 'Script to run K-Means in MonetDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--columns', nargs='*', type=int, default = [100],
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=str, default='2021-01-31', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2021-02-01', help='')
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.udf_template = os.path.abspath('udf_template.sql')
args.implementation_path = os.path.abspath('../../../Algorithms/kmeans/')
args.db_dir = os.path.abspath('../master_farm')
args.output_path = os.path.abspath('output_clusters.csv')

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

                print("Generating udf")
                column_types = "d0 DOUBLE PRECISION"
                for i in range(1, columns):
                        column_types = column_types + ", d" + str(i) + " DOUBLE PRECISION"
                column_names = "d0"
                for i in range(1, columns):
                        column_names = column_names + ", d" + str(i)
                f = open(args.udf_template, "r")
                g = open(args.udf_template + ".sql", "w")
                for l in f.readlines():
                        g.write( l.replace("<column_types>", column_types)
				  .replace("<column_names>", column_names)
				  .replace("<lines>", str(lines))
				  .replace("<columns>", str(columns))
				  .replace("<start_time>", args.start_time)
				  .replace("<end_time>", args.end_time)
				  .replace("<data_file>", args.file + ".csv")
				  .replace("<db_dir>", args.db_dir)
				  .replace("<implementation_path>", args.implementation_path)
				  .replace("<output_path>", args.output_path) )
                f.close()
                g.close()

                os.system("mclient -p54320 -d mydb " + args.udf_template + ".sql")
                os.system("rm " + args.udf_template + ".sql")
                os.system("rm " + args.file + ".csv")