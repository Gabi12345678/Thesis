CREATE TABLE datapoints (t TIMESTAMP, d DOUBLE);
CREATE TABLE metrics_insert (current_time DOUBLE);

CREATE TABLE result_sum (d DOUBLE);
CREATE TABLE metrics_sum (current_time DOUBLE);

CREATE TABLE result_interval (t TIMESTAMP, d DOUBLE);
CREATE TABLE metrics_interval (current_time DOUBLE);

CREATE TABLE result_moving (t TIMESTAMP, d DOUBLE);
CREATE TABLE metrics_moving (current_time DOUBLE);

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
	
	f = open("<input_file>", "r")

	lines = <lines>
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = float(line[1])
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))

	return "insert_data"
';

INSERT INTO metrics_insert VALUES (current_time());
-- import datapoints <input_file>;
SELECT import_data();
INSERT INTO metrics_insert VALUES (current_time());
SELECT  MAX(current_time) - MIN(current_time) AS insert_time, 
	<lines>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_values_per_second
FROM metrics_insert;

INSERT INTO metrics_sum VALUES(current_time());
INSERT INTO result_sum SELECT SUM(d) FROM datapoints;
INSERT INTO metrics_sum VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as SUM_time FROM metrics_sum;

INSERT INTO metrics_interval VALUES(current_time());
INSERT INTO result_interval SELECT t, d FROM datapoints WHERE t > cast('<start_time>' as timestamp) AND t < cast('<end_time>' as timestamp);
INSERT INTO metrics_interval VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as INTERVAL_time FROM metrics_interval;

INSERT INTO metrics_moving VALUES(current_time());
INSERT INTO result_moving SELECT t, AVG(d) OVER (ORDER BY t ASC RANGE BETWEEN 1440 PRECEDING AND 0 FOLLOWING) FROM datapoints;
INSERT INTO metrics_moving VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as MOVING_time FROM metrics_moving;
