from datetime import datetime
from tqdm import tqdm
import argparse
import os
import subprocess
import time
import urllib.request
from pydruid.client import *

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

parser = argparse.ArgumentParser(description = 'Script to run simple queries in eXtremeDB')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default=[100], help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--column', nargs='?', type=int, default=38, help='list of integers representing the number of lines to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=str, default='2021-01-31', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2021-02-01', help='')
parser.add_argument('--moving_average_hours', nargs='?', type=int, default=24, help='')
args = parser.parse_args()

args.druid_path=os.path.abspath('../apache-druid-0.19.0/bin/start-micro-quickstart')
args.import_script=os.path.abspath('../apache-druid-0.19.0/bin/post-index-task')
args.import_config=os.path.abspath('./import-config.json')
args.template_import_config=os.path.abspath('./template-import-config.json')
args.storage_dir=os.path.abspath('../apache-druid-0.19.0/var')
args.data_file=os.path.abspath('./data.json')
args.base_dir=os.path.abspath('./')

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
	print("Deleting previous data")
#	subprocess.Popen(["rm", args.storage_dir, "-R"], stdout = subprocess.DEVNULL)

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
	generate_from_template(args.template_import_config, args.import_config, 
				[("<base_dir>", args.base_dir)])

	f = open(args.file, "r")
	g = open(args.data_file, "w")
	for i in range(lines):
		values = f.readline().split(" ")
		currentTime = get_datetime(values[0])[0]
		datapoint = { "time": currentTime.isoformat() + ".000Z" }
		datapoint["dim"] = float(values[args.column + 1])
		g.write(json.dumps(datapoint).replace(" ", "") + "\n")
	g.close()
	f.close()

	initial_size = get_size()
	initial_time = get_time()
	import_task = subprocess.Popen([args.import_script, "--file", args.import_config, "--url", "http://localhost:8081"], stdout = subprocess.DEVNULL)
	import_task.wait()
	final_time = get_time()
	final_size = get_size()
	print("*" * 100)
	print("(lines) =", (lines))
	print("Insert time:", final_time - initial_time)
	print("Total size (bytes):", final_size - initial_size)
	print("Total size (MB):", (final_size - initial_size) / 1024.0 / 1024.0)
	print("Throughput (I/sec):", 1.0 * lines / (final_time - initial_time))
	print("Throughput (V/sec):", 1.0 * lines / (final_time - initial_time))
	print("*" * 100)

	initial_time_udf = get_time()
	query = PyDruid("http://localhost:8083", 'druid/v2')
	ts = query.timeseries(datasource = 'master', 
			granularity = 'all', 
			intervals = ['2010-01-01/2030-01-01'], 
			aggregations = { "clusters": {"type": "doubleSum", "fieldName": "dim"}})
	final_time_udf = get_time()
	print(ts.export_pandas())
	print("*" * 100)
	print("Sum time:", final_time_udf - initial_time_udf)
	print("*" * 100)

	interval_t1 = int( (datetime.strptime(args.start_time, '%Y-%m-%d') - datetime(1970, 1, 1)).total_seconds() )
	interval_t2 = int( (datetime.strptime(args.end_time, '%Y-%m-%d') - datetime(1970, 1, 1)).total_seconds() )
	initial_time_udf = get_time()
	query = PyDruid("http://localhost:8083", 'druid/v2')
	ts = query.scan(datasource = 'master',
		granularity = 'all',
		intervals = [args.start_time + '/' + args.end_time])
	final_time_udf = get_time()
	print("*" * 100)
	print("Interval time:", final_time_udf - initial_time_udf)
	print("*" * 100)

#	initial_time_udf = get_time()
#	query = PyDruid("http://localhost:8083", 'druid/v2')
#	q = query.query_builder.build_query(
#		query_type = 'movingAverage',
#		args = {
#			"datasource": 'master',
#			"granularity": 'minute',
#			"intervals": [args.start_time + '/' + args.end_time],
#			"aggregations": { "d": { "type": "doubleSum", "fieldName": "dim" } },
#			"averagers": [ {
#				"name": "movingMean",
#				"fieldName": "d",
#				"type": "doubleMeanNoNulls",
#				"buckets": 10
#			} ] 
#		}
#	)
#	ts = query._post(q)
#	final_time_udf = get_time()
#	print(ts.export_pandas())
#	print("*" * 100)
#	print("Moving time:", final_time_udf - initial_time_udf)
#	print("*" * 100)

	print("Terminating druid")
	druid.terminate()
	druid.wait()

subprocess.Popen(["rm", args.import_config]).wait()
subprocess.Popen(["rm", args.data_file]).wait()
