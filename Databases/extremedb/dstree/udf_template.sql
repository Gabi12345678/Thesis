CREATE TABLE datapoints (t TIMESTAMP, d ARRAY(DOUBLE));
CREATE TABLE distances (t TIMESTAMP, dist DOUBLE);
CREATE TABLE metrics_insert (current_time DOUBLE, disk_usage INTEGER);
CREATE TABLE metrics_index_builder(current_time DOUBLE);
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
	return(datetime.now() - datetime(1970, 1, 1)).total_seconds()
';

create function index_builder() RETURNS string in 'python' as '
	import sys
	sys.path.append("<implementation_path>")
	from dstree.dynamicsplit import IndexBuilder

	IndexBuilder.buildIndex("<data_file>", "<index_path>", 100, 1, 1000, <columns>)
	return "Done"
';

create function udf() RETURNS string in 'python' as '
	import sys
	import numpy as np
	from datetime import datetime
	sys.path.append("<implementation_path>")
	from dstree.dynamicsplit import IndexExactSearcher

	exdb.init_runtime(skip_load=True)

	cur = current_session.cursor()
	cur.execute("SELECT * FROM datapoints")
	results = cur.fetchall()

	lines = len(results)
	columns = len(results[0]) - 1
	matrix = []

	for i in range(lines):
		current_line = []
		for j in range(columns):
			current_line.append(results[i][j+1])
		current_line = np.array(current_line)
		matrix.append(current_line)
	matrix = np.array(matrix).T

	distances = IndexExactSearcher.search(matrix, "<index_path>.idx_dyn_100_1_<columns>")

	start_epoch = <start_time>
	for i in range(len(distances)):
		time = datetime.fromtimestamp(start_epoch + i * 10)
		cur.execute("INSERT INTO distances VALUES (?, ?)", (time, distances[i]) )
	return "success"
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

	f = open("<query_file>", "r")

	lines = <lines>
	columns = <columns>
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))

	return "success"
';

INSERT INTO metrics_insert VALUES (current_time(), disk_usage());
select import_data();
INSERT INTO metrics_insert VALUES (current_time(), disk_usage());
SELECT  MAX(disk_usage) - MIN(disk_usage) AS disk_usage,
	MAX(current_time) - MIN(current_time) AS insert_time,
	<lines>.0 / (MAX(current_time) - MIN(current_time)) AS Throughput_inserts_per_second,
	<lines>.0 * <columns>.0 / (MAX(current_time) - MIN(current_time)) AS Throughput_values_per_second
FROM metrics_insert;

INSERT INTO metrics_index_builder VALUES(current_time());
SELECT index_builder();
INSERT INTO metrics_index_builder VALUES(current_time());
SELECT MAX(current_time) - MIN(current_time) AS index_builder_time FROM metrics_index_builder;

INSERT INTO metrics_udf VALUES(current_time());
SELECT udf();
INSERT INTO metrics_udf VALUES(current_time());
SELECT MAX(current_time) - MIN(current_time) AS search_time FROM metrics_udf;
