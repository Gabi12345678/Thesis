CREATE TABLE datapoints (t TIMESTAMP, <column_types>);
CREATE TABLE clusters (t TIMESTAMP, <column_types>);
CREATE TABLE metrics_insert (current_time DOUBLE, disk_usage INTEGER);
CREATE TABLE metrics_udf (current_time DOUBLE);

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
	import kmeans

	exdb.init_runtime(skip_load=True)
	
	cur = current_session.cursor()
	cur.execute("SELECT * FROM datapoints")
	results = cur.fetchall()
	
	lines = <lines>
	columns = <columns>
	matrix = []
	
	for i in range(lines):
		current_line = []
		for j in range(columns):
			current_line.append(results[i][j+1])
		current_line = np.array(current_line)
		matrix.append(current_line)
	matrix = np.array(matrix)

	clusters = kmeans.kmeans(matrix, 10, 20)

	start_epoch = <start_time>
	for i in range(len(clusters)):
		time = datetime.fromtimestamp(start_epoch + i * 10)
		cur.execute("INSERT INTO clusters(t) VALUES (?)", (time,) )
		for j in range(0, columns, 50):
			start_interval = j
			end_interval = min(columns, start_interval + 50)
			update_sql = "UPDATE clusters SET"
			for t in range(start_interval, end_interval):
				update_sql = update_sql + " d" + str(t) + " = ?,"
			update_sql = update_sql[:-1] + " WHERE t = ?"
			cur.execute(update_sql, tuple(clusters[i][start_interval:end_interval]) + (time,))

	return "success"
';

create function import_data() RETURNS string in 'python' as '
	import datetime
	exdb.init_runtime(skip_load = True)
	cur = current_session.cursor()
	
	f = open("<input_file>", "r")

	lines = <lines>
	columns = <columns>
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = datetime.datetime.strptime(line[0], "%Y-%m-%d %H:%M:%S")
		current_line = []
		for j in range(columns):
			current_line.append( float(line[j + 1]) )
		cur.execute("INSERT INTO datapoints(t) VALUES (?)", (time, ))
		
		for j in range(0, columns, 50):
			start_interval = j
			end_interval = min(columns, start_interval + 50)
			update_sql = "UPDATE datapoints SET"
			for t in range(start_interval, end_interval):
				update_sql = update_sql + " d" + str(t) + " = ?,"
			update_sql = update_sql[:-1] + " WHERE t = ?"
			cur.execute(update_sql, tuple(current_line[start_interval:end_interval]) + (time, ) )
	
	return "success"
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
SELECT MAX(current_time) - MIN (current_time) as kmeans_time FROM metrics_udf;
