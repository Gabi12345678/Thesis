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

parser = argparse.ArgumentParser(description = 'Script to run ZScore in Kairos')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synth_1K.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--columns', nargs='*', type=int, default = [100],
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=int,
        help='epoch time of the first datasample. All others will be set at 10 second intervals',
        default=1583000000)
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
	for columns in args.columns:
		initial_size = 0
		print("Deleting previous data")
		try:
			subprocess.check_output("rm " + args.queue_dir + " -R", shell=True)
		except subprocess.CalledProcessError:
			pass
		try:
			Cluster(['localhost']).connect('kairosdb').execute('drop keyspace kairosdb;')
			print("Data deleted.")
		except NoHostAvailable:
			print("No previous data exists. Skipping delete.")
		print("Starting Kairosdb")
		kairos = subprocess.Popen([args.kairos_path, "run"], stderr = subprocess.DEVNULL, stdout = subprocess.DEVNULL)
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
		print("Generating data for (" + str(lines) + ", " + str(columns) + ")")
		
		initial_time = current_time()
		f = open(args.file, "r")
		for i in tqdm(range(lines)):
			values = f.readline()[:-1].split(" ")
			t = (args.start_time + i * 10) * 1000
			data = []
			for j in range(columns):
				data.append({
					"name": "master.data",
					"datapoints": [[t, values[j]]],
					"tags": {
						"dim": "dim" + str(j)
					}
				})
			r = requests.post("http://localhost:8080/api/v1/datapoints", data = json.dumps(data))
		print("Waiting for data to be commited")
		session = Cluster(['localhost']).connect('kairosdb')
		cnt = [r for r in session.execute('select count(*) from data_points;')][0].count
		print(cnt)
		while cnt != lines * columns:
			time.sleep(0.01)
			cnt = [r for r in session.execute('select count(*) from data_points;')][0].count
			print(cnt)
		final_time = current_time()

		dataSave = {
			"start_absolute": (args.start_time - 10) * 1000,
			"end_absolute": (args.start_time + 10 + 10 * lines) * 1000, 
			"metrics": [ 
				{ 
					"name": "master.data", 
					"aggregators": [
						{ 
							"name": "zscore", 
							"lines": lines, 
							"columns": columns
						}, { 
							"name": "save_as", 
							"metric_name": "master.data.result"
						}, {
							"name": "filter", 
							"filter_op": "LTE", 
							"threshold": "0.0"
						}, {
							"name": "filter", 
							"filter_op": "GTE", 
							"threshold": "0.0"
						}
					]
				} 
			]
		}
		
		initial_time_udf = current_time()
		r = requests.post("http://localhost:8080/api/v1/datapoints/query", data=json.dumps(dataSave))
		final_time_udf = current_time()
		print("Terminating kairos")
		kairos.terminate()
		kairos.wait()
		final_size = get_size()

		print("*" * 100)
		print("(lines, columns) =", (lines, columns))
		print("Insert time:", final_time - initial_time)
		print("Total size (bytes):", final_size - initial_size)
		print("Total size (MB):", (final_size - initial_size) / 1024.0 / 1024.0)
		print("Throughput (I/sec):", 1.0 * lines / (final_time - initial_time))
		print("Throughput (V/sec):", 1.0 * lines * columns / (final_time - initial_time))
		print("KMeans time:", final_time_udf - initial_time_udf)
		print("*" * 100)
		
		subprocess.check_output("rm " + args.queue_dir + " -R", shell=True)