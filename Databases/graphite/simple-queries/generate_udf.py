from datetime import datetime
from tqdm import tqdm
import argparse
import os
import socket
import time
import requests

def get_datetime(s):
    from datetime import datetime
    try:
        return (datetime.strptime(s, "%Y-%m-%dT%H:%M:%S"), "seconds")
    except ValueError:
        pass
    try:
        f = datetime.strptime(s, "%Y-%m-%dT%H:%M")
        if f < datetime(2020, 11, 1):
            return (f, "minutes.long")
        else:
            return (f, "minutes.short")
    except ValueError:
        pass
    return (datetime.strptime(s, "%Y-%m-%d"), "days")

parser = argparse.ArgumentParser(description = 'Script to run K-Means in Graphite')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/alabama_weather.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--column', nargs='*', type=int, default = 38,
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=str, default='2014-01-01', help='')
parser.add_argument('--end_time', nargs='?', type=str, default='2019-01-01', help='')
parser.add_argument('--moving_average_hours', nargs='?', type=int, default=24, help='')
args = parser.parse_args()

args.storage_path = os.path.abspath("../storage/whisper/master")


for lines in args.lines:
	print("Deleting previous data")
	return_value = os.system("sudo rm -rf " + args.storage_path)
	while return_value != 0:
		print("Error while deleting. Retrying...")
		return_value = os.system("sudo rm -rf " + args.storage_path)
	print("Adding data for (lines) = (" + str(lines) + ")")

	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect(("localhost", 2003))
	sock.setblocking(1)

	f = open(args.file, "r")
	t1 = 10**12
	t2 = -1
	for i in tqdm(range(lines)):
		values = f.readline()[:-1].split(" ")
		currentTime, args.format = get_datetime(values[0])
		currentTime = (currentTime - datetime(1970, 1, 1)).total_seconds()
		currentTime = int(currentTime)
		t1 = min(t1, currentTime)
		t2 = max(t2, currentTime)
		insert_value = "master." + args.format + ".v " + values[args.column + 1] + " " + str(currentTime) + "\n"
		sock.sendall( bytes(insert_value, "UTF-8") )
	f.close()
	while True:
		try:
			initial_last_modified = os.path.getmtime(args.storage_path + "/" + args.format.replace(".", "/"))
		except FileNotFoundError:
			continue
		break
	time.sleep(60)
	current_last_modified = os.path.getmtime(args.storage_path + "/" + args.format.replace(".", "/"))
	while current_last_modified != initial_last_modified:
		initial_last_modified = current_last_modified
		time.sleep(60)
		current_last_modified = os.path.getmtime(args.storage_path + "/" + args.format.replace(".", "/"))
	sock.close()
	
	initial_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()	

	params = {
		'from': t1 - 20,
		'until': t2 + 20,
		'format': 'csv',
		'target': 'smartSummarize(master.' + args.format + '.v, "10y", "sum")',
	}	
	r = requests.get(url = "http://localhost/render", params = params)
	if not(r.ok):
		print(r.text)
	final_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()
	print("+" * 100)
	print("Sum time: ", final_time - initial_time)
	print("+" * 100)

	interval_t1 = int( (datetime.strptime(args.start_time, '%Y-%m-%d') - datetime(1970, 1, 1)).total_seconds() )
	interval_t2 = int( (datetime.strptime(args.end_time, '%Y-%m-%d') - datetime(1970, 1, 1)).total_seconds() )
	params = {
		'from': interval_t1,
		'until': interval_t2,
		'format': 'csv',
		'target': 'master.' + args.format + '.v',
	}
	r = requests.get(url = "http://localhost/render", params = params)
	if not(r.ok):
		print(r.text)
	final_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()
	print("+" * 100)
	print("Interval time: ", final_time - initial_time)
	print("+" * 100)

	params = {
		'from': t1 - 20,
		'until': t2 + 20,
		'format': 'csv',
		'target': 'movingAverage(master.' + args.format + '.v, "' + str(args.moving_average_hours) + 'h")',
	}
	r = requests.get(url = "http://localhost/render", params = params)
	if not(r.ok):
		print(r.text)
	final_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()
	print("+" * 100)
	print("Moving time: ", final_time - initial_time)
	print("+" * 100)

print("Cleaning up data")
return_value = os.system("sudo rm -rf " + args.storage_path)
while return_value != 0:
	print("Error while cleaning up. Retrying...")
	return_value = os.system("sudo rm -rf " + args.storage_path)
