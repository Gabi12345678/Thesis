CREATE TABLE datapoints_seq ( ind INTEGER PRIMARY KEY, d SEQUENCE(DOUBLE) );
CREATE TABLE zscore_seq ( ind INTEGER PRIMARY KEY, d SEQUENCE(DOUBLE) );

CREATE TABLE metrics_udf (current_time DOUBLE);

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
	import numpy as np

	exdb.init_runtime(skip_load = True)
	cur = current_session.cursor()
	
	f = open("<input_file>", "r")

	lines = <lines>
	columns = <columns>
	matrix = []
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		matrix.append( data )
	matrix = np.array(matrix).T.tolist()
	for i in range(columns):
		cur.execute("INSERT INTO datapoints_seq VALUES (?, ?)", (i, matrix[i]))
	return "success"
';

SELECT import_data();
--SELECT * FROM datapoints_seq;

INSERT INTO metrics_udf VALUES(current_time());
INSERT INTO zscore_seq SELECT ind, (d - seq_avg(d)) / seq_dev(d) FROM datapoints_seq;
INSERT INTO metrics_udf VALUES(current_time());

--SELECT * from zscore_seq;
SELECT MAX(current_time) - MIN (current_time) as zscore_time FROM metrics_udf;
--SELECT * FROM zscore;
