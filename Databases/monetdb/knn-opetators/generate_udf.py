from datetime import datetime
from tqdm import tqdm
import argparse
import random
import os

parser = argparse.ArgumentParser(description = 'Script to run K-Means in MonetDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
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
                g = open(args.file + "_labeled.csv", "w")
                h = open(args.file + "_unlabeled.csv", "w")
                read_lines = [""] * (2 * lines)
                for i in tqdm(range(lines * 2)):
                        read_lines[i] = f.readline()
                for i in tqdm(range(lines)):
                        unlabeled_values = read_lines[i][:-1].split(" ")
                        unlabeled_values = unlabeled_values[:columns + 1]
                        h.write(",".join(unlabeled_values) + "\n")
                        label_values = read_lines[i + lines][:-1].split(" ")[:columns + 1]
                        label_values = label_values + [str(random.randrange(5))]
                        g.write(",".join(label_values) + "\n")
                f.close()
                g.close()
                h.close()

                print("Generating udf")
                column_types = "d0 DOUBLE PRECISION"
                for i in range(1, columns):
                        column_types = column_types + ", d" + str(i) + " DOUBLE PRECISION"
                distance_format = "SQRT( POWER(a{0} - b{0}, 2) )"
                distance_formula = distance_format.format(0)
                for i in range(1, columns):
                        distance_formula = distance_formula + " + " + distance_format.format(i)
                dist_arg_format = "a{0} DOUBLE PRECISION, b{0} DOUBLE PRECISION"
                dist_arg = dist_arg_format.format(0)
                for i in range(1, columns):
                        dist_arg = dist_arg + ", " + dist_arg_format.format(i)
                dist_param_format = "ldatapoints.d{0}, udatapoints.d{0}"
                dist_param = dist_param_format.format(0)
                for i in range(1, columns):
                        dist_param = dist_param + ", " + dist_param_format.format(i)
                f = open(args.udf_template, "r")
                g = open(args.udf_template + ".sql", "w")
                for l in f.readlines():
                        g.write( l.replace("<column_types>", column_types)
				  #.replace("<unlabel_column_names>", unlabel_column_names)
				  #.replace("<label_column_names>", label_column_names)
				  .replace("<lines>", str(lines))
				  .replace("<columns>", str(columns))
				  .replace("<data_file_labeled>", args.file + "_labeled.csv")
                                  .replace("<data_file_unlabeled>", args.file + "_unlabeled.csv")
				  .replace("<db_dir>", args.db_dir)
				  .replace("<implementation_path>", args.implementation_path)
				  .replace("<distance_formula>", distance_formula)
				  .replace("<dist_arg>", dist_arg)
				  .replace("<dist_param>", dist_param)
				  .replace("<output_path>", args.output_path) )
                f.close()
                g.close()

                os.system("mclient -p54320 -d mydb " + args.udf_template + ".sql")
                #os.system("rm " + args.udf_template + ".sql")
                #os.system("rm " + args.file + ".csv")
