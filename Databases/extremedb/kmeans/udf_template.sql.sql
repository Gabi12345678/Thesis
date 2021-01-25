CREATE TABLE datapoints (t TIMESTAMP, d ARRAY(DOUBLE));
CREATE TABLE clusters (t TIMESTAMP, d ARRAY(DOUBLE));
CREATE TABLE metrics_insert (current_time DOUBLE, disk_usage INTEGER);
CREATE TABLE metrics_udf (current_time DOUBLE);

create function disk_usage() returns integer in 'python' as '
	import os
        def get_size(start_path = "/home/gabi/Thesis-master/Databases/extremedb/kmeans"):
                total_size = 0
                for dirpath, dirnames, filenames in os.walk(start_path):
                        for f in filenames:
                                fp = os.path.join(dirpath, f)
                                # skip if it is symbolic link
                                if not os.path.islink(fp):
                                        total_size += os.path.getsize(fp)
                return total_size
        return get_size()
';

create function current_time() returns double in 'python' as '
	from datetime import datetime
	return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
';

create function udf() RETURNS string in 'python' as '
	import sys
	import numpy as np
	from datetime import datetime
	sys.path.append("/home/gabi/Thesis-master/Algorithms/kmeans")
	import kmeans

	exdb.init_runtime(skip_load=True)
	
	cur = current_session.cursor()
	cur.execute("SELECT t, d FROM datapoints")
	results = cur.fetchall()
	
	lines = 1000000
	columns = 46
	matrix = []
	timestamps = []
	for i in range(lines):
		matrix.append( np.array( results[i][1] ))
		timestamps.append( results[i][0] )

	matrix = np.array(matrix)

	clusters = kmeans.kmeans(matrix, 10, 20)

	for i in range(len(clusters)):
		cur.execute("INSERT INTO clusters(t, d) VALUES (?, ?)", (timestamps[i], tuple(clusters[i])))

	return "udf"
';

create function import_data() RETURNS string in 'python' as '
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

	import datetime
	exdb.init_runtime(skip_load = True)
	cur = current_session.cursor()
	
	f = open("/home/gabi/Thesis-master/Datasets/alabama_weather.txt.csv", "r")

	lines = 1000000
	columns = 46
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))

	return "insert_data"
';

INSERT INTO metrics_insert VALUES (current_time(), disk_usage());
-- import datapoints /home/gabi/Thesis-master/Datasets/alabama_weather.txt.csv;
SELECT import_data();
INSERT INTO metrics_insert VALUES (current_time(), disk_usage());
SELECT  MAX(disk_usage) - MIN(disk_usage) AS disk_usage, 
	MAX(current_time) - MIN(current_time) AS insert_time, 
	1000000.0 / (MAX(current_time) - MIN(current_time)) as Throughput_inserts_per_second,
	1000000.0 * 46.0 / (MAX(current_time) - MIN(current_time)) as Throughput_values_per_second 
FROM metrics_insert;

INSERT INTO metrics_udf VALUES(current_time());
SELECT udf();
INSERT INTO metrics_udf VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as kmeans_time FROM metrics_udf;
