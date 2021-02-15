CREATE TABLE datapoints (t TIMESTAMP, d ARRAY(DOUBLE));

CREATE TABLE result_select (t TIMESTAMP, d ARRAY(DOUBLE));
CREATE TABLE metrics_select (current_time DOUBLE);

CREATE TABLE result_moving (t TIMESTAMP, d ARRAY(DOUBLE));
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
	columns = <columns>
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))

	return "insert_data"
';

SELECT import_data();

INSERT INTO metrics_select VALUES(current_time());
INSERT INTO result_select SELECT t, d FROM datapoints WHERE t > cast('<start_time>' as timestamp) AND t < cast('<end_time>' as timestamp);
INSERT INTO metrics_select VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as SELECT_time FROM metrics_select;

select count(*) from result_select;
