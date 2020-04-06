import os
from tqdm import tqdm
from influxdb import InfluxDBClient
from datetime import datetime

def get_size(start_path = '/home/gabi/.influxdb'):
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(start_path):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            # skip if it is symbolic link
            if not os.path.islink(fp):
                total_size += os.path.getsize(fp)
    return total_size

client = InfluxDBClient(database="master")
client.drop_database("master")
initial_size = get_size()
client.create_database('master')

path = "../dataset/synth_1M.txt"
lines = 100
columns = 6

f = open(path, "r")
l = []

initial_time = datetime.now()

for x in tqdm(range(lines)):
        s = f.readline()[:-1]
        s = s.split(" ")
        v = {}
        for y in range(columns):
                v["dim" + str(y)] = float(s[y])
        body = {"measurement": "puncte", "time":1583000000000000000+10000000000*x,  "fields": v }
        l.append(body)
        if len(l) == 100:
                client.write_points(l)
                l=[]
client.write_points(l)

final_time = datetime.now()
seconds = (final_time - initial_time).total_seconds()

final_size = get_size()
print("memory: ", final_size-initial_size)
print("throughput vps: ", lines/seconds*columns)
print("thoughput: ", lines/seconds)