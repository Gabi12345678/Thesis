from datetime import datetime
from tqdm import tqdm
import argparse
import os
import socket
import time
import requests

parser = argparse.ArgumentParser(description = 'Script to run K-Means in Graphite')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synth_1K.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--columns', nargs='*', type=int, default = [100],
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 20 --columns 4" will try (20, 4)')
parser.add_argument('--start_time', nargs='?', type=int,
        help='epoch time of the first datasample. All others will be set at 10 second intervals',
        default=1583000000)
args = parser.parse_args()

args.storage_path = os.path.abspath("../storage/whisper/master")


def get_size(start_path = args.storage_path):
	total_size = 0
	for dirpath, dirnames, filenames in os.walk(start_path):
		for f in filenames:
			fp = os.path.join(dirpath, f)
			# skip if it is symbolic link
			if not os.path.islink(fp):
				total_size += os.path.getsize(fp)
	return total_size



for lines in args.lines:
	for columns in args.columns:
		print("Deleting previous data")
		return_value = os.system("sudo rm -rf " + args.storage_path)
		while return_value != 0:
			print("Error while deleting. Retrying...")
			return_value = os.system("sudo rm -rf " + args.storage_path)
		print("Adding data for (lines, columns) = (" + str(lines) + ", " + str(columns) + ")")

		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.connect(("localhost", 2003))
		sock.setblocking(1)

		initial_size = get_size()
		initial_time = (datetime.utcnow() - datetime(1970, 1, 1)).total_seconds()

		f = open(args.file, "r")
		for i in tqdm(range(lines)):
			values = f.readline()[:-1].split(" ")
			currentTime = args.start_time + i * 10
			for j in range(columns):
				insert_value = "master.dim" + str(j) + " " + values[j] + " " + str(currentTime) + "\n"
				sock.sendall( bytes(insert_value, "UTF-8") )
		f.close()
		sock.close()

		print("Waiting for Graphite to commit data to disk...")
		while True:
			try:
				initial_last_modified = os.path.getmtime(args.storage_path)
			except FileNotFoundError:
				continue
			break
		time.sleep(30)
		current_last_modified = os.path.getmtime(args.storage_path)
		while current_last_modified != initial_last_modified:
			initial_last_modified = current_last_modified
			time.sleep(30)
			current_last_modified = os.path.getmtime(args.storage_path)

		final_size = get_size()
		final_time = current_last_modified

		print("+" * 100)
		print("Total size bytes: ", final_size - initial_size)
		print("Total size megabytes: ", float(final_size - initial_size) / 1024.0 / 1024.0)
		print("Total time in seconds: ", final_time - initial_time)
		print("Throughput inserts per second: ", float(lines) / (final_time - initial_time))
		print("Throughput valuess per second: ", float(lines * columns) / (final_time - initial_time))
		print("+" * 100)

		initial_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()	

		params = {
			'from': args.start_time - 20,
			'until': args.start_time + (lines + 2) * 10,
			'format': 'csv',
			'target': 'kmeans(master.dim*)',
		}
		
		requests.get(url = "http://localhost/render", params = params)
		final_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()
		print("+" * 100)
		print("KMeans time: ", final_time - initial_time)
		print("+" * 100)

print("Cleaning up data")
return_value = os.system("sudo rm -rf " + args.storage_path)
while return_value != 0:
	print("Error while cleaning up. Retrying...")
	return_value = os.system("sudo rm -rf " + args.storage_path)
