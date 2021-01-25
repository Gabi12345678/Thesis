import subprocess
from datetime import datetime
from tqdm import tqdm
from influxdb import InfluxDBClient
import argparse
import os
import numpy as np
import time
import socket
import sys

def get_datetime(s):
     from datetime import datetime
     try:
         return (datetime.strptime(s, "%Y-%m-%dT%H:%M:%S"), "seconds")
     except ValueError:
         pass
     try:
         return (datetime.strptime(s, "%Y-%m-%dT%H:%M"), "minutes")
     except ValueError:
         pass
     return (datetime.strptime(s, "%Y-%m-%d"), "days")

# Arguments
parser = argparse.ArgumentParser(description = 'Script to run Similarity Search in Influx')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/hydraulic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 30" (20, 30)')
parser.add_argument('--columns', nargs='*', type=int, default=[100],
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 30" will try (20, 30)')
parser.add_argument('--start_time', nargs='?', type=int, help = "Epoch time in second for the first measurement. All others will be 10 seconds apart", default=1583000000)

args = parser.parse_args()

args.home_path = os.path.expanduser("~")
args.db_directory = os.path.abspath(args.home_path + '/.influxdb')
args.kapacitor_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitor')
args.kapacitord_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitord')
args.kapacitor_config_template_path = os.path.abspath("template_kapacitor.config")
args.kapacitor_config_path = os.path.abspath('kapacitor.config')
args.dstree_handler_template_path = os.path.abspath('template_dstree_handler.py')
args.dstree_handler_path = os.path.abspath('dstree_handler.py')
args.influxd_path = os.path.abspath('../influxdb-1.7.10-1/usr/bin/influxd')
args.tick_path = os.path.abspath('udf.tick')
args.implementation_path = os.path.abspath('../../../Algorithms/dstree_python/')
args.kapacitor_library = os.path.abspath('../kapacitor-master/udf/agent/py')
args.output_file = os.path.abspath('time.txt')
args.udf_template = os.path.abspath('udf_template.sql')
args.file = os.path.abspath(args.file)
args.index_dir = os.path.abspath("./out")
args.index_path = os.path.abspath('./out/index')

sys.path.append(args.implementation_path)
from dstree.dynamicsplit import IndexBuilder

def generate_from_template(template_path, file_path, changes):
        f = open(template_path, "r")
        g = open(file_path, "w")
        for line in f.readlines():
                new_line = line
                for before, after in changes:
                        new_line = new_line.replace(before, after)
                g.write(new_line)
        f.close()
        g.close()

# Function to get database storage roon
def get_size(start_path = args.db_directory):
	total_size = 0
	for dirpath, dirnames, filenames in os.walk(start_path):
		for f in filenames:
			fp = os.path.join(dirpath, f)
			# skip if it is symbolic link
			if not os.path.islink(fp):
				total_size += os.path.getsize(fp)
	return total_size

generate_from_template(template_path = args.kapacitor_config_template_path, file_path = args.kapacitor_config_path,
			changes = [("<handler_path>", args.dstree_handler_path), ("<home_path>", args.home_path)])
generate_from_template(template_path = args.dstree_handler_template_path, file_path = args.dstree_handler_path,
				changes = [("<implementation>", args.implementation_path), ("<kapacitor_library>", args.kapacitor_library),
						("<output_file>", args.output_file), ("<index_path>", args.index_path)])
influx = subprocess.Popen([args.influxd_path], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
while True:
	try:
		subprocess.check_output("curl localhost:8086", shell=True, stderr = subprocess.DEVNULL)
	except subprocess.CalledProcessError:
		print("Influx is not ready. Checking again in 4 seconds")
		time.sleep(4)
		continue
	break
print("Influx is ready to accept data")
kapacitor = subprocess.Popen([args.kapacitord_path, "-config", args.kapacitor_config_path], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
while True:
	try:
		subprocess.check_output("curl localhost:9092", shell=True, stderr = subprocess.DEVNULL)
	except subprocess.CalledProcessError:
		print("Kapacitor is not yet ready. Checking again in 10 seconds.")
		time.sleep(4)
		continue
	break


for line in args.lines:
	for column in args.columns:
		print("Generating data file")
		f = open(args.file, "r")
		g = open(args.file + ".csv", "w")
		h = open(args.file + "_index.txt", "w")
		index_lines = []
		data_lines = []
		for i in tqdm(range(line)):
			values = f.readline()[:-1].split(" ")
			data_lines.append( values[ : (column + 1)] )
			index_lines.append( values[(column + 1) : (2 * column + 1) ] )
		index_lines = np.array(index_lines).T.tolist()
		for i in tqdm(range(column)):
			h.write(" ".join(index_lines[i]) + "\n")
		f.close()
		g.close()
		h.close()

		print("Generating index")
		IndexBuilder.buildIndex(args.file + "_index.txt", args.index_path, 100, 1, 1000, column)

		print("Running UDF for (" + str(line) + ", " + str(column) + ")")

		client = InfluxDBClient(database = 'master')
		client.drop_database('master')

		initial_size = get_size()
		client.create_database('master')
		initial_time = datetime.now()

		l = []
		bucket_size = 100
		if line > 10000 or column > 10000:
			bucket_size = 10
		for x in tqdm(range(line)):
			s = data_lines[x]
			v = {}
			for y in range(column):
				v["dim" + str(y)] = float(s[y + 1])
			time = (get_datetime(s[0])[0] - datetime(1970, 1, 1)).total_seconds() * 1000000000
			time = int(time)
			body = {"measurement": "puncte", "time": time, "fields": v }
			l.append(body)
			if len(l) == bucket_size:
				client.write_points(l)
				l = []
		client.write_points(l)

		final_time = datetime.now()
		final_size = get_size()

		seconds = (final_time - initial_time).total_seconds()

		print("Memory in bytes: ", final_size - initial_size)
		print("Memory in megabytes: ", float(final_size - initial_size) / 1024.0 / 1024.0)
		print("Througput: ", line / seconds)

		subprocess.run([args.kapacitor_path, "define", "udf", "-tick", "udf.tick"])
		subprocess.run([args.kapacitor_path, "enable", "udf"])
		subprocess.run([args.kapacitor_path, "delete", "replays", "udf-replay"])
		subprocess.run([args.kapacitor_path, "delete", "recordings", "udf-recording"])
		subprocess.run([args.kapacitor_path, "record", "batch", "-past", "300d", "-recording-id", "udf-recording", "-task", "udf"])
		subprocess.run([args.kapacitor_path, "replay", "-recording", "udf-recording", "-replay-id", "udf-replay", "-task", "udf"])
		
	
print("Terminating kapacitor")
kapacitor.terminate()
print("Terminating influx")
influx.terminate()

subprocess.run(["rm", args.kapacitor_config_path])
subprocess.run(["rm", args.dstree_handler_path])
os.system("rm ./out/* -r")
