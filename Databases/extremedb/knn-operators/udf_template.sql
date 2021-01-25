CREATE TABLE datapoints (t TIMESTAMP, d ARRAY(DOUBLE));
CREATE TABLE label_datapoints (t TIMESTAMP, l INTEGER, d ARRAY(DOUBLE));
CREATE TABLE metrics_insert (current_time DOUBLE);
CREATE TABLE metrics_udf (current_time DOUBLE);
CREATE TABLE result (t TIMESTAMP, l INTEGER);

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
		data = [float(x) for x in line[1:]]
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))
	
	g = open("<label_data_file>", "r")

	for i in range(lines):
		line = g.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:-1]]
		label = int(line[-1])
		cur.execute("INSERT INTO label_datapoints(t, l, d) VALUES (?, ?, ?)", (time, label, data))

	return "insert_data"
';

create function knn_alg() RETURNS string  IN 'python' as '

	from Queue import PriorityQueue
	exdb.init_runtime(skip_load = True)
	cur = current_session.cursor()

	cur.execute("SELECT a.t, b.l, sqrt(<distance_formula>) AS dist FROM datapoints as a, label_datapoints as b")
	a = cur.fetchall()
	q = {}
	for i in range(len(a)):
		current_time = a[i][0]
		if not(current_time in q):
			q[current_time] = PriorityQueue()
		q[current_time].put( (-a[i][2], a[i][1]) )
		while q[current_time].qsize() > 3:
			q[current_time].get()

	for time in q:
		a = q[time]
		l_size = a.qsize()
		l = [0] * l_size
		for i in range(l_size):
			l[i] = a.get()[1]
		cur.execute("INSERT INTO result(t, l) VALUES (?, ?)", (time, max(set(l), key = l.count)))
	return "success"
';

INSERT INTO metrics_insert VALUES (current_time());
-- import datapoints <input_file>;
SELECT import_data();
INSERT INTO metrics_insert VALUES (current_time());
SELECT 	MAX(current_time) - MIN(current_time) AS insert_time, 
	<lines>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_inserts_per_second,
	<lines>.0 * <columns>.0 / (MAX(current_time) - MIN(current_time)) as Throughput_values_per_second 
FROM metrics_insert;

INSERT INTO metrics_udf VALUES(current_time());
SELECT knn_alg();
--SELECT a.d, b.d, sqrt(<distance_formula>) FROM datapoints as a, label_datapoints as b;
INSERT INTO metrics_udf VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as knn_time FROM metrics_udf;
--SELECT * FROM result;
