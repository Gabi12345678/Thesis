import subprocess
from datetime import datetime
from tqdm import tqdm
from influxdb import InfluxDBClient
import influxdb
import argparse
import os
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
parser = argparse.ArgumentParser(description = 'Script to run CentroidDecomposition in Influx')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/alabama_weather.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 30" (20, 30)')
parser.add_argument('--columns', nargs='*', type=int, default=[100],
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 30" will try (20, 30)')
parser.add_argument('--start_time', nargs='?', type=int, help = "Epoch time in second for the first measurement. All others will be 10 seconds apart", default=1583000000)

args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.home_path = os.path.expanduser("~")
args.db_directory = os.path.abspath(args.home_path + '/.influxdb')
args.kapacitor_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitor')
args.kapacitord_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitord')
args.kapacitor_config_template_path = os.path.abspath("template_kapacitor.config")
args.kapacitor_config_path = os.path.abspath('kapacitor.config')
args.cd_handler_template_path = os.path.abspath('template_cd_handler.py')
args.cd_handler_path = os.path.abspath('cd_handler.py')
args.influxd_path = os.path.abspath('../influxdb-1.7.10-1/usr/bin/influxd')
args.tick_path = os.path.abspath('udf.tick')
args.implementation_path = os.path.abspath('../../../Algorithms/centroid_decomposition/')
args.kapacitor_library = os.path.abspath('../kapacitor-master/udf/agent/py')
args.output_file = os.path.abspath('time.txt')

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

generate_from_template(template_path = args.kapacitor_config_template_path, file_path = args.kapacitor_config_path,
                        changes = [("<handler_path>", args.cd_handler_path), ("<home_path>", args.home_path)])
generate_from_template(template_path = args.cd_handler_template_path, file_path = args.cd_handler_path,
			changes = [("<implementation>", args.implementation_path), ("<kapacitor_library>", args.kapacitor_library), ("<output_file>", args.output_file)])

influx = subprocess.Popen([args.influxd_path], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
while True:
	try:
		subprocess.check_output("curl localhost:8086", shell=True, stderr = subprocess.DEVNULL)
	except subprocess.CalledProcessError:
		print("Influx is not ready. Checking again in 10 seconds")
		time.sleep(10)
		continue
	break
print("Influx is ready to accept data.")

kapacitor = subprocess.Popen([args.kapacitord_path, "-config", args.kapacitor_config_path], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
while True:
	try:
		subprocess.check_output("curl localhost:9092", shell=True, stderr = subprocess.DEVNULL)
	except subprocess.CalledProcessError:
		print("Kapacitor is not yet ready. Checking again in 10 seconds.")
		time.sleep(10)
		continue
	break
print("Influx is ready to accept data.")

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

for line in args.lines:
	for column in args.columns:
		print("Running UDF for (" + str(line) + ", " + str(column) + ")")

		client = InfluxDBClient(database = 'master')
		client.drop_database('master')

		initial_size = get_size()
		client.create_database('master')
		initial_time = datetime.now()

		f = open(args.file, "r")
		l = []
		bucket_size = 10
		if line > 10000:
			bucket_size = 10
		for x in tqdm(range(line)):
			s = f.readline()[:-1].split(" ")
			v = {}
			for y in range(column):
				v["dim" + str(y)] = float(s[y + 1])
			currentTime = get_datetime(s[0])[0]
			time = (currentTime - datetime(1970, 1, 1)).total_seconds() * 1000000000
			time = int(time)
			body = {"measurement": "puncte", "time": time, "fields": v }
			l.append(body)
			if len(l) == 100:
				while True:
					try:
						client.write_points(l)
					except influxdb.exceptions.InfluxDBServerError:
						continue
					break
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
		print("OK")
		subprocess.run([args.kapacitor_path, "record", "batch", "-past", "4000d", "-recording-id", "udf-recording", "-task", "udf"])
		subprocess.run([args.kapacitor_path, "replay", "-recording", "udf-recording", "-replay-id", "udf-replay", "-task", "udf"])
	
print("Terminating kapacitor")
kapacitor.terminate()
print("Terminating influx")
influx.terminate()

subprocess.run(["rm", args.kapacitor_config_path])
subprocess.run(["rm", args.cd_handler_path])
