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

parser = argparse.ArgumentParser(description = 'Script to run Euclidean Distance in Graphite')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../../Datasets/synthetic.txt')
parser.add_argument('--lines', nargs='*', type=int, default = [100],
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 --columns 4" will try (10, 4)')
parser.add_argument('--columns', nargs='*', type=int, default = [10],
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

		f = open(args.file, "r")
		t1 = 10**12
		t2 = -1
		values = []
		for i in tqdm(range(lines)):
			values.append( f.readline()[:-1].split(" ") )
			currentTime, args.format = get_datetime(values[i][0])
			currentTime = (currentTime - datetime(1970, 1, 1)).total_seconds()
			currentTime = int(currentTime)
			t1 = min(t1, currentTime)
			t2 = max(t2, currentTime)
		for i in tqdm(range(lines)):
			values.append( f.readline()[:-1].split(" ") )
		f.close()
		
		bucket_size = 1000
		total_time = 0.0
		initial_time = (datetime.utcnow() - datetime(1970, 1, 1)).total_seconds()
		for j in tqdm(range(columns)):
			for i in range(lines):
				currentTime, args.format = get_datetime(values[i][0])
				currentTime = (currentTime - datetime(1970, 1, 1)).total_seconds()
				currentTime = int(currentTime)
				insert_value = "master." + args.format + ".a" + str(j) + " " + values[i][j + 1] + " " + str(currentTime) + "\n"
				sock.sendall( bytes(insert_value, "UTF-8") )
				insert_value = "master." + args.format + ".b" + str(j) + " " + values[i + lines][j + 1] + " " + str(currentTime) + "\n"
				sock.sendall( bytes(insert_value, "UTF-8") )
			if (j + 1) % bucket_size == 0 or j == columns - 1:	
				while True:
					try:
						initial_last_modified = os.path.getmtime(args.storage_path + "/" + args.format.replace(".", "/"))
					except FileNotFoundError:
						continue
					break
				time.sleep(60)
				current_last_modified = os.path.getmtime(args.storage_path + "/" + args.format.replace(".", "/"))
				print(current_last_modified)
				while current_last_modified != initial_last_modified:
					initial_last_modified = current_last_modified
					time.sleep(60)
					current_last_modified = os.path.getmtime(args.storage_path + "/" + args.format.replace(".", "/"))
					print(current_last_modified)
				total_time = total_time + (current_last_modified - initial_time)
				initial_time = (datetime.utcnow() - datetime(1970, 1, 1)).total_seconds()
		sock.close()
			
		final_size = get_size()

		print("+" * 100)
		print("Total size bytes: ", final_size - initial_size)
		print("Total size megabytes: ", float(final_size - initial_size) / 1024.0 / 1024.0)
		print("Total time in seconds: ", total_time)
		print("Throughput inserts per second: ", float(lines) / total_time)
		print("Throughput valuess per second: ", float(lines * columns) / total_time)
		print("+" * 100)

		dist_format = 'pow(diffSeries(master.{0}.a{1}, master.{0}.b{1}),2)'
		dist_exp = dist_format.format(args.format, 0)
		for i in range(1, columns):
			dist_exp = dist_exp + "," + dist_format.format(args.format, i)
		target = 'squareRoot(sumSeries(' + dist_exp + '))'

		initial_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()	

		params = {
			'from': t1 - 20,
			'until': t2 + 20,
			'format': 'csv',
			'target': target,
		}
		
		r = requests.get(url = "http://localhost/render", params = params)
		final_time = (datetime.now() - datetime(1970, 1, 1)).total_seconds()
		q = open("out.txt", "w")
		q.write(r.text)
		q.close()
		print("+" * 100)
		print("Dist time: ", final_time - initial_time)
		print("+" * 100)

print("Cleaning up data")
#return_value = os.system("sudo rm -rf " + args.storage_path)
return_value = 0
while return_value != 0:
	print("Error while cleaning up. Retrying...")
	return_value = os.system("sudo rm -rf " + args.storage_path)
