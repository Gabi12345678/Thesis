CREATE TABLE datapoints (t TIMESTAMP, d ARRAY(DOUBLE));
CREATE TABLE label_datapoints (t TIMESTAMP, d ARRAY(DOUBLE), l INTEGER);
CREATE TABLE metrics_insert (current_time DOUBLE, disk_usage INTEGER);
CREATE TABLE metrics_udf (current_time DOUBLE);
CREATE TABLE lresult (l INTEGER);

create function disk_usage() returns integer in 'python' as '
	import os
        def get_size(start_path = "<db_dir>"):
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
	sys.path.append("<implementation_path>")
	import knn

	exdb.init_runtime(skip_load=True)
	
	cur = current_session.cursor()
	cur.execute("SELECT t, d FROM datapoints")
	results = cur.fetchall()
	
	lines = <lines>
	columns = <columns>
	matrix = []
	for i in range(lines):
		matrix.append( np.array( results[i][1] ))
	matrix = np.array(matrix)

	cur.execute("SELECT t, d, l FROM label_datapoints")
	results = cur.fetchall()
	label_matrix = []
	labels = []
	for i in range(lines):
		label_matrix.append( np.array(results[i][1]) )
		labels.append( results[i][2] )
	label_matrix = np.array(label_matrix)

	result = knn.knn(label_matrix, labels, matrix, 3)

	sql = """INSERT INTO lresult (l) VALUES (?)"""
	for i in range(len(result)):
		cur.execute(sql, (result[i],))

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
	import random

	exdb.init_runtime(skip_load = True)
	cur = current_session.cursor()
	
	f = open("<input_file>", "r")

	lines = <lines>
	columns = <columns>
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))
	
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		label = random.randrange(5)
		cur.execute("INSERT INTO label_datapoints(t, d, l) VALUES (?, ?, ?)", (time, data, label))

	return "insert_data"
';

INSERT INTO metrics_insert VALUES (current_time(), disk_usage());
-- import datapoints <input_file>;
SELECT import_data();
INSERT INTO metrics_insert VALUES (current_time(), disk_usage());
SELECT  MAX(disk_usage) - MIN(disk_usage) AS disk_usage, 
	MAX(current_time) - MIN(current_time) AS insert_time, 
	<lines>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_inserts_per_second,
	<lines>.0 * <columns>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_values_per_second 
FROM metrics_insert;

INSERT INTO metrics_udf VALUES(current_time());
SELECT udf();
INSERT INTO metrics_udf VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as knn_time FROM metrics_udf;
