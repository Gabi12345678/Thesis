CREATE TABLE datapoints (t TIMESTAMP, a ARRAY(DOUBLE), b ARRAY(DOUBLE));
CREATE TABLE metrics_insert (current_time DOUBLE);
CREATE TABLE metrics_udf (current_time DOUBLE);
CREATE TABLE result (t TIMESTAMP, d DOUBLE);

create function current_time() returns double in 'python' as '
	from datetime import datetime
	return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
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
	
	f = open("<data_file>", "r")

	lines = <lines>
	columns = <columns>
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		a = [float(x) for x in line[1:(columns + 1)]]
		b = [float(x) for x in line[columns + 1:]]
		cur.execute("INSERT INTO datapoints(t, a, b) VALUES (?, ?, ?)", (time, a, b))
	
	return "insert_data"
';


INSERT INTO metrics_insert VALUES (current_time());
SELECT import_data();
INSERT INTO metrics_insert VALUES (current_time());
SELECT 	MAX(current_time) - MIN(current_time) AS insert_time, 
	<lines>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_inserts_per_second,
	<lines>.0 * <columns>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_values_per_second 
FROM metrics_insert;

INSERT INTO metrics_udf VALUES(current_time());
INSERT INTO result SELECT t, sqrt(<distance_formula>) FROM datapoints;
INSERT INTO metrics_udf VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as knn_time FROM metrics_udf;

--SELECT * FROM datapoints;
--SELECT * FROM result;
