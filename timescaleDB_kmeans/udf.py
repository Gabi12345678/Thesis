from tqdm import tqdm
import random
from datetime import datetime
import psycopg2
import sys
import os

lines = 10
columns = 10
clusters = 10
data_path = '/home/gabi/influx/dataset/synth_1M.txt'
start_time = 1583000000

def get_size(start_path = '/var/lib/postgresql/11/main'):
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(start_path):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            # skip if it is symbolic link
            if not os.path.islink(fp):
                total_size += os.path.getsize(fp)
    return total_size

# -----------------------------------------------------------------------------------------
# Clear previous database
print("Clear previous database")
# Create connection to database
con = psycopg2.connect(database = "gabi", user = "gabi", password = "gabi")
cursor  = con.cursor()

cursor.execute("DROP FUNCTION IF EXISTS kmeans;")
cursor.execute("DROP TYPE IF EXISTS datapoint CASCADE;")
cursor.close()
con.commit()
con.close()
# Clear previous database
# -----------------------------------------------------------------------------------------

initial_size = get_size()
initial_time = datetime.now()

# -----------------------------------------------------------------------------------------
# Write db
print("Write db")
# Re-create the connection since we closed the last one.
con = psycopg2.connect(database = "gabi", user = "gabi", password = "gabi")
cursor  = con.cursor()

# Create the database type.
type_sql = "CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, cluster_id NUMERIC"
for i in range(columns):
        type_sql = type_sql + ", d" + str(i) + " double precision"
type_sql = type_sql + ");"

# Create the database table.
cursor.execute(type_sql)
cursor.execute("CREATE TABLE datapoints OF datapoint WITH (fillfactor=10);")
cursor.execute("CREATE TABLE clusters OF datapoint WITH (fillfactor=10);")
cursor.execute("CREATE extension IF NOT EXISTS timescaledb CASCADE;")
cursor.execute("SELECT create_hypertable('datapoints', 'time');")
cursor.execute("SELECT create_hypertable('clusters', 'time');");

# Insert values from file, one by one.
# Also select a subset of the lines for the initial clusters.
cluster_index = random.sample(range(lines), clusters)
d = open(data_path, "r")
for i in tqdm(range(lines)):
        values = d.readline()[:-1].split(" ")
        time = datetime.fromtimestamp(start_time + i * 10).strftime("%Y-%m-%d %H:%M:%S")
        insert_sql = "('" + time + "', " + str(i)
        for j in range(columns):
                insert_sql = insert_sql + ", " + values[j][:10]
        insert_sql = insert_sql + ");"
        cursor.execute("INSERT INTO datapoints VALUES " + insert_sql )
        # If value was selected to be cluster center, then also add it in clusters.
        if i in cluster_index:
                cursor.execute("INSERT INTO clusters VALUES " + insert_sql)

cursor.close()
con.commit()
con.close()
# Write db
# ------------------------------------------------------------------------------------------

final_time = datetime.now();
final_size = get_size();
time_in_seconds = (final_time - initial_time).total_seconds();

print("Total size (bytes): ", final_size - initial_size)
print("Total size (Megabytes): ", (final_size - initial_size) / 1024 / 1024)
print("Throughput (inserts / second): ", lines / time_in_seconds)
print("Throughput (values / second): ", lines * columns / time_in_seconds)