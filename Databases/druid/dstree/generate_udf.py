from datetime import datetime
from tqdm import tqdm
import argparse
import os
import numpy as np
import subprocess
import time
import urllib.request
from pydruid.client import *

parser = argparse.ArgumentParser(description = 'Script to run DStree in Druid')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100], help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--columns', nargs='*', type=int, default=[40], help='list of integers representing the number of lines to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=int, help='epoch time of the first datasample. All other will be set at 10 seconds intervals', default=1583000000)
args = parser.parse_args()

args.druid_path=os.path.abspath('../apache-druid-0.19.0/bin/start-micro-quickstart')
args.import_script=os.path.abspath('../apache-druid-0.19.0/bin/post-index-task')
args.import_config=os.path.abspath('./import-config.json')
args.template_import_config=os.path.abspath('./template-import-config.json')
args.storage_dir=os.path.abspath('../apache-druid-0.19.0/var')
args.data_json=os.path.abspath('./data.json')
args.base_dir=os.path.abspath('./')
args.file = os.path.abspath(args.file)
args.index_path = os.path.abspath('./out/index')

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

def get_size(start_path = args.storage_dir):
	total_size = 0
	for dirpath, dirnames, filenames in os.walk(start_path):
		for f in filenames:
			fp = os.path.join(dirpath, f)
			# skip if it is symbolic link
			if not os.path.islink(fp):
				total_size += os.path.getsize(fp)
	return total_size

def get_time():
	return (datetime.now() - datetime(1970, 1, 1)).total_seconds()

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

for lines in args.lines:
	for columns in args.columns:
		print("Deleting previous data")
		subprocess.Popen(["rm", args.storage_dir, "-R"], stdout = subprocess.DEVNULL)

		print("Starting Druid")
		druid = subprocess.Popen([args.druid_path], stdout = subprocess.DEVNULL)
		while True:
			try:
				subprocess.check_output("curl localhost:8081", shell=True, stderr = subprocess.DEVNULL)
				subprocess.check_output("curl localhost:8082", shell=True, stderr = subprocess.DEVNULL)
				subprocess.check_output("curl localhost:8083", shell=True, stderr = subprocess.DEVNULL)
			except:
				print("Druid is not ready. Checking again in 4 seconds")
				time.sleep(4)
				continue
			break
		print("Druid is ready")
		
		print("Inserting data")
		dims = [ "dim" + str(i) for i in range(columns) ]
		dim_types = [ "{ \"name\": \"dim" + str(i) + "\", \"type\": \"double\" }" for i in range(columns)]
		dim_types = ',\n          '.join(dim_types)
		generate_from_template(args.template_import_config, args.import_config, 
					[("<base_dir>", args.base_dir), ("<dim_types>", dim_types)])

		f = open(args.file, "r")
		g = open(args.data_json, "w")
		h = open(args.file + "_index.txt", "w")
		index_lines = []
		for i in tqdm(range(lines)):
			values = f.readline().split(" ")
			t = get_datetime(values[0])[0]
			datapoint = { "time": t.isoformat() + ".000Z" }
			for j in range(columns):
				datapoint["dim" + str(j)] = float(values[j + 1])
			g.write(json.dumps(datapoint).replace(" ", "") + "\n")
			index_lines.append( values[(columns + 1) : (2 * columns + 1)] )
		index_lines = np.array(index_lines).T.tolist()
		for i in tqdm(range(columns)):
			h.write(" ".join(index_lines[i]) + "\n")
		f.close()
		g.close()
		h.close()

		initial_size = get_size()
		initial_time = get_time()
		import_task = subprocess.Popen([args.import_script, "--file", args.import_config, "--url", "http://localhost:8081"], stdout = subprocess.DEVNULL)
		import_task.wait()
		final_time = get_time()
		final_size = get_size()

		print("Indexing")
		initial_time_index = get_time()
		query = PyDruid("http://localhost:8083", 'druid/v2')
		ts = query.timeseries(datasource = 'master', 
				granularity = 'year', 
				intervals = ['2020-01-01/2030-01-01'], 
				aggregations = { "indexrezult": {"type": "dstreeindex", "datafile": args.file + "_index.txt", "indexfile": args.index_path, "tscount": lines}})
		final_time_index = get_time()
		print(ts.export_pandas()["indexrezult"][0])

		print("Searching")
		initial_time_udf = get_time()
		query = PyDruid("http://localhost:8083", 'druid/v2')
		ts = query.timeseries(datasource = 'master',
					granularity = 'year',
					intervals = ['2020-01-01/2030-01-01'],
					aggregations = { "searchresult": { "type": "dstreesearch", "indexfile": args.index_path + ".idx_dyn_100_1_" + str(lines), "columns": dims, "timeField": "__time"}})
		final_time_udf = get_time()
		#print(ts.export_pandas()["searchresult"][0])

		print("Terminating druid")
		druid.terminate()
		druid.wait()

		print("*" * 100)
		print("(lines, columns) =", (lines, columns))
		print("Insert time:", final_time - initial_time)
		print("Total size (bytes):", final_size - initial_size)
		print("Total size (MB):", (final_size - initial_size) / 1024.0 / 1024.0)
		print("Throughput (I/sec):", 1.0 * lines / (final_time - initial_time))
		print("Throughput (V/sec):", 1.0 * lines * columns / (final_time - initial_time))
		print("Index time:", final_time_index - initial_time_index)
		print("Search time:", final_time_udf - initial_time_udf)
		print("*" * 100)

#subprocess.Popen(["rm", args.import_config]).wait()
#subprocess.Popen(["rm", args.data_file]).wait()
