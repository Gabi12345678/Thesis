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
import math

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
parser = argparse.ArgumentParser(description = 'Script to run K-Means in Influx')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/hydraulic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--columns', nargs='*', type=int, default=[100],
	help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.home_path = os.path.expanduser("~")
args.db_directory = os.path.abspath(args.home_path + '/.influxdb')
args.kapacitor_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitor')
args.kapacitord_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitord')
args.influxd_path = os.path.abspath('../influxdb-1.7.10-1/usr/bin/influxd')
args.tick_path = os.path.abspath('udf.tick')

influx = subprocess.Popen([args.influxd_path], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
while True:
	try:
		subprocess.check_output("curl localhost:8086", shell=True, stderr = subprocess.DEVNULL)
	except subprocess.CalledProcessError:
		print("Influx is not yet ready. Checking again in 10 seconds.")
		time.sleep(10)
		continue
	break
print("Influx is ready to accept data.")
kapacitor = subprocess.Popen([args.kapacitord_path], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
while True:
	try:
		subprocess.check_output("curl localhost:9092", shell=True, stderr = subprocess.DEVNULL)
	except subprocess.CalledProcessError:
		print("Kapacitor is not yet ready. Checking again in 10 seconds.")
		time.sleep(10)
		continue
	break
print("Kapacitor is ready to accept data.")

for lines in args.lines:
	for columns in args.columns:
		print("Running UDF for (" + str(lines) + ")")

		client = InfluxDBClient(database = 'master')
		client.drop_database('master')
		
		client.create_database('master')

		f = open(args.file, "r")
		l = []
		bucket_size = 100
		if lines > 10000 or columns > 10000:
			bucket_size = 10
		bucket_size = 10
		for x in tqdm(range(lines)):
			s = f.readline()[:-1].split(" ")
			v = {}
			for y in range(columns):
				v["d" + str(y)] = float(s[y+1])
			time = (get_datetime(s[0])[0] - datetime(1970, 1, 1)).total_seconds() * 1000000000
			time = int(time)
			body = {"measurement": "puncte", "time": time, "fields": v }
			l.append(body)
			if len(l) == bucket_size:
				while True:
					try:
						client.write_points(l)
					except influxdb.exceptions.InfluxDBServerError:
						print(sys.exc_info())
						continue
					break
				l = []
		client.write_points(l)

		t1 = datetime.now()
		rs = client.query("SELECT mean(*), stddev(*) FROM puncte")
		rs = list(rs.get_points())[0]
		stddev_factor = math.sqrt(lines - 1) / math.sqrt(lines)
		select_format = "(d{0} - {1:.10f}) / {2:.10f}"
		select_query = "SELECT " + select_format.format(0, rs["mean_d0"], rs["stddev_d0"] * stddev_factor)
		for i in range(1, columns):
			select_query = select_query + ", " + select_format.format(i, rs["mean_d" + str(i)], rs["stddev_d"+str(i)] * stddev_factor)
		select_query = select_query + " FROM puncte"
		rs = client.query(select_query)
		t2 = datetime.now()
		print("Total ZScore time: ", (t2 - t1).total_seconds())
		break

print("Terminating kapacitor")
kapacitor.terminate()	

print("Terminating influx")
influx.terminate()
