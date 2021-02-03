from datetime import datetime
from tqdm import tqdm
import argparse
import requests
import os
import json
import time
import subprocess
import time
from datetime import datetime
from cassandra.cluster import Cluster, NoHostAvailable

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

parser = argparse.ArgumentParser(description = 'Script to run KMeans in Kairos')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--column', nargs='?', type=int, default = 38,
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=str, default='2021-01-31', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2021-02-01', help='')
parser.add_argument('--moving_average_hours', nargs='?', type=int, default=24, help='')


args = parser.parse_args()
args.db_dir = os.path.abspath("../data/data/kairosdb/")
args.kairos_path = os.path.abspath("../kairosdb/bin/kairosdb.sh")
args.queue_dir = os.path.abspath("../kairosdb/queue")

def get_size(start_path = args.db_dir):
	total_size = 0
	for dirpath, dirnames, filenames in os.walk(start_path):
		for f in filenames:
			fp = os.path.join(dirpath, f)
			# skip if it is symbolic link
			if not os.path.islink(fp):
				total_size += os.path.getsize(fp)
	return total_size
def current_time():
	return (datetime.now() - datetime(1970, 1, 1)).total_seconds()


for lines in args.lines:
	initial_size = 0
	print("Deleting previous data")
	try:
		subprocess.check_output("rm " + args.queue_dir + " -R", shell=True)
	except subprocess.CalledProcessError:
		pass
	try:
		session = Cluster(['localhost']).connect('kairosdb')
		session.default_timeout = 1200
		session.execute('drop keyspace kairosdb;')
		print("Data deleted.")
	except NoHostAvailable:
		print("No previous data exists. Skipping delete")
	print("Starting Kairosdb")
	kairos = subprocess.Popen([args.kairos_path, "run"], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
	time.sleep(4)
	while True:
		try:
			subprocess.check_output("curl localhost:8080", shell=True, stderr = subprocess.DEVNULL)
		except subprocess.CalledProcessError:
			print("Kairos is not ready yet. Checking again in 2 seconds.")
			time.sleep(2)
			continue
		break
	print("Kairos is ready")
	print("Generating data for (" + str(lines) +  ")")
	initial_time = current_time()
	start_interval = 10**12
	end_interval = 0
	f = open(args.file, "r")
	for i in tqdm(range(lines)):
		values = f.readline()[:-1].split(" ")
		t = (get_datetime(values[0])[0] - datetime(1970, 1, 1)).total_seconds() * 1000
		t = int(t)
		start_interval = min(start_interval, t)
		end_interval = max(end_interval, t)
		data = []
		data.append({
			"name": "master.data",
			"datapoints": [[t, values[args.column + 1]]],
			"tags": {
				"data": "master"
			}
		})
		r = requests.post("http://localhost:8080/api/v1/datapoints", data = json.dumps(data))
	print("Waiting for data to be commited")
	session = Cluster(['localhost']).connect('kairosdb')
	session.default_timeout = 1200
	cnt = [r for r in session.execute('select count(*) from data_points;')][0].count
	while cnt < lines:
		time.sleep(0.01)
		cnt = [r for r in session.execute('select count(*) from data_points;')][0].count
	final_time = current_time()

	print("*" * 100)
	print("(lines) =", (lines))
	print("Insert time:", final_time - initial_time)
	print("Throughput (V/sec):", 1.0 * lines / (final_time - initial_time))
	print("*" * 100)
	dataSave = {
		"start_absolute": start_interval - 10000,
		"end_absolute": end_interval + 10000,
		"metrics": [
			{
				"name": "master.data",
				"aggregators": [
					{
						"name": "sum",
						"sampling": {
							"value": int((end_interval - start_interval + 20000) / 1000),
							"unit": "seconds"
						}
					}
				]
			}
		]
	}
	initial_time_udf = current_time()
	r = requests.post("http://localhost:8080/api/v1/datapoints/query", data=json.dumps(dataSave))
	final_time_udf = current_time()
	print("*" * 100)
	#print(r.text)
	print("SUM time:", final_time_udf - initial_time_udf)
	print("*" * 100)
	interval_t1 = int( (datetime.strptime(args.start_time, '%Y-%m-%d') - datetime(1970, 1, 1)).total_seconds() ) * 1000
	interval_t2 = int( (datetime.strptime(args.end_time, '%Y-%m-%d') - datetime(1970, 1, 1)).total_seconds() ) * 1000
	dataSave = {
		"start_absolute": interval_t1,
		"end_absolute": interval_t2,
		"metrics": [
			{
				"name": "master.data",
			}
		]
	}
	initial_time_udf = current_time()
	r = requests.post("http://localhost:8080/api/v1/datapoints/query", data=json.dumps(dataSave))
	if not(r.ok):
		print(r.text)
	final_time_udf = current_time()
	print("*" * 100)
	print("Interval time:", final_time_udf - initial_time_udf)
	print("*" * 100)

	dataSave = {
		"start_absolute": start_interval - 10000,
		"end_absolute": end_interval + 10000,
		"metrics": [
			{
				"name": "master.data",
				"aggregators": [
					{
						"name": "sma",
						"size": args.moving_average_hours * 60
					}
				]
			}
		]
	}
	initial_time_udf = current_time()
	r = requests.post("http://localhost:8080/api/v1/datapoints/query", data=json.dumps(dataSave))
	final_time_udf = current_time()
	if not(r.ok):
		print(r.text)
	print("*" * 100)
	print("Moving time:", final_time_udf - initial_time_udf)
	print("*" * 100)

	print("Terminating kairos")
	kairos.terminate()
	kairos.wait()

	subprocess.check_output("rm " + args.queue_dir + " -R", shell=True)
