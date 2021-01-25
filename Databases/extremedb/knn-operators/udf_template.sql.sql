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
	
	f = open("/home/gabi/Thesis-master/Datasets/alabama_weather.txt.csv", "r")

	lines = 10000
	columns = 46
	for i in range(lines):
		line = f.readline()[:-1].split(",")
		time = get_datetime(line[0])[0]
		data = [float(x) for x in line[1:]]
		cur.execute("INSERT INTO datapoints(t, d) VALUES (?, ?)", (time, data))
	
	g = open("/home/gabi/Thesis-master/Datasets/alabama_weather.txt_label.csv", "r")

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

	cur.execute("SELECT a.t, b.l, sqrt((a.d[0] - b.d[0])*(a.d[0] - b.d[0]) + (a.d[1] - b.d[1])*(a.d[1] - b.d[1]) + (a.d[2] - b.d[2])*(a.d[2] - b.d[2]) + (a.d[3] - b.d[3])*(a.d[3] - b.d[3]) + (a.d[4] - b.d[4])*(a.d[4] - b.d[4]) + (a.d[5] - b.d[5])*(a.d[5] - b.d[5]) + (a.d[6] - b.d[6])*(a.d[6] - b.d[6]) + (a.d[7] - b.d[7])*(a.d[7] - b.d[7]) + (a.d[8] - b.d[8])*(a.d[8] - b.d[8]) + (a.d[9] - b.d[9])*(a.d[9] - b.d[9]) + (a.d[10] - b.d[10])*(a.d[10] - b.d[10]) + (a.d[11] - b.d[11])*(a.d[11] - b.d[11]) + (a.d[12] - b.d[12])*(a.d[12] - b.d[12]) + (a.d[13] - b.d[13])*(a.d[13] - b.d[13]) + (a.d[14] - b.d[14])*(a.d[14] - b.d[14]) + (a.d[15] - b.d[15])*(a.d[15] - b.d[15]) + (a.d[16] - b.d[16])*(a.d[16] - b.d[16]) + (a.d[17] - b.d[17])*(a.d[17] - b.d[17]) + (a.d[18] - b.d[18])*(a.d[18] - b.d[18]) + (a.d[19] - b.d[19])*(a.d[19] - b.d[19]) + (a.d[20] - b.d[20])*(a.d[20] - b.d[20]) + (a.d[21] - b.d[21])*(a.d[21] - b.d[21]) + (a.d[22] - b.d[22])*(a.d[22] - b.d[22]) + (a.d[23] - b.d[23])*(a.d[23] - b.d[23]) + (a.d[24] - b.d[24])*(a.d[24] - b.d[24]) + (a.d[25] - b.d[25])*(a.d[25] - b.d[25]) + (a.d[26] - b.d[26])*(a.d[26] - b.d[26]) + (a.d[27] - b.d[27])*(a.d[27] - b.d[27]) + (a.d[28] - b.d[28])*(a.d[28] - b.d[28]) + (a.d[29] - b.d[29])*(a.d[29] - b.d[29]) + (a.d[30] - b.d[30])*(a.d[30] - b.d[30]) + (a.d[31] - b.d[31])*(a.d[31] - b.d[31]) + (a.d[32] - b.d[32])*(a.d[32] - b.d[32]) + (a.d[33] - b.d[33])*(a.d[33] - b.d[33]) + (a.d[34] - b.d[34])*(a.d[34] - b.d[34]) + (a.d[35] - b.d[35])*(a.d[35] - b.d[35]) + (a.d[36] - b.d[36])*(a.d[36] - b.d[36]) + (a.d[37] - b.d[37])*(a.d[37] - b.d[37]) + (a.d[38] - b.d[38])*(a.d[38] - b.d[38]) + (a.d[39] - b.d[39])*(a.d[39] - b.d[39]) + (a.d[40] - b.d[40])*(a.d[40] - b.d[40]) + (a.d[41] - b.d[41])*(a.d[41] - b.d[41]) + (a.d[42] - b.d[42])*(a.d[42] - b.d[42]) + (a.d[43] - b.d[43])*(a.d[43] - b.d[43]) + (a.d[44] - b.d[44])*(a.d[44] - b.d[44]) + (a.d[45] - b.d[45])*(a.d[45] - b.d[45])) AS dist FROM datapoints as a, label_datapoints as b")
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
	10000.0 / (MAX(current_time) - MIN(current_time)) as Throughput_inserts_per_second,
	10000.0 * 46.0 / (MAX(current_time) - MIN(current_time)) as Throughput_values_per_second 
FROM metrics_insert;

INSERT INTO metrics_udf VALUES(current_time());
SELECT knn_alg();
--SELECT a.d, b.d, sqrt((a.d[0] - b.d[0])*(a.d[0] - b.d[0]) + (a.d[1] - b.d[1])*(a.d[1] - b.d[1]) + (a.d[2] - b.d[2])*(a.d[2] - b.d[2]) + (a.d[3] - b.d[3])*(a.d[3] - b.d[3]) + (a.d[4] - b.d[4])*(a.d[4] - b.d[4]) + (a.d[5] - b.d[5])*(a.d[5] - b.d[5]) + (a.d[6] - b.d[6])*(a.d[6] - b.d[6]) + (a.d[7] - b.d[7])*(a.d[7] - b.d[7]) + (a.d[8] - b.d[8])*(a.d[8] - b.d[8]) + (a.d[9] - b.d[9])*(a.d[9] - b.d[9]) + (a.d[10] - b.d[10])*(a.d[10] - b.d[10]) + (a.d[11] - b.d[11])*(a.d[11] - b.d[11]) + (a.d[12] - b.d[12])*(a.d[12] - b.d[12]) + (a.d[13] - b.d[13])*(a.d[13] - b.d[13]) + (a.d[14] - b.d[14])*(a.d[14] - b.d[14]) + (a.d[15] - b.d[15])*(a.d[15] - b.d[15]) + (a.d[16] - b.d[16])*(a.d[16] - b.d[16]) + (a.d[17] - b.d[17])*(a.d[17] - b.d[17]) + (a.d[18] - b.d[18])*(a.d[18] - b.d[18]) + (a.d[19] - b.d[19])*(a.d[19] - b.d[19]) + (a.d[20] - b.d[20])*(a.d[20] - b.d[20]) + (a.d[21] - b.d[21])*(a.d[21] - b.d[21]) + (a.d[22] - b.d[22])*(a.d[22] - b.d[22]) + (a.d[23] - b.d[23])*(a.d[23] - b.d[23]) + (a.d[24] - b.d[24])*(a.d[24] - b.d[24]) + (a.d[25] - b.d[25])*(a.d[25] - b.d[25]) + (a.d[26] - b.d[26])*(a.d[26] - b.d[26]) + (a.d[27] - b.d[27])*(a.d[27] - b.d[27]) + (a.d[28] - b.d[28])*(a.d[28] - b.d[28]) + (a.d[29] - b.d[29])*(a.d[29] - b.d[29]) + (a.d[30] - b.d[30])*(a.d[30] - b.d[30]) + (a.d[31] - b.d[31])*(a.d[31] - b.d[31]) + (a.d[32] - b.d[32])*(a.d[32] - b.d[32]) + (a.d[33] - b.d[33])*(a.d[33] - b.d[33]) + (a.d[34] - b.d[34])*(a.d[34] - b.d[34]) + (a.d[35] - b.d[35])*(a.d[35] - b.d[35]) + (a.d[36] - b.d[36])*(a.d[36] - b.d[36]) + (a.d[37] - b.d[37])*(a.d[37] - b.d[37]) + (a.d[38] - b.d[38])*(a.d[38] - b.d[38]) + (a.d[39] - b.d[39])*(a.d[39] - b.d[39]) + (a.d[40] - b.d[40])*(a.d[40] - b.d[40]) + (a.d[41] - b.d[41])*(a.d[41] - b.d[41]) + (a.d[42] - b.d[42])*(a.d[42] - b.d[42]) + (a.d[43] - b.d[43])*(a.d[43] - b.d[43]) + (a.d[44] - b.d[44])*(a.d[44] - b.d[44]) + (a.d[45] - b.d[45])*(a.d[45] - b.d[45])) FROM datapoints as a, label_datapoints as b;
INSERT INTO metrics_udf VALUES(current_time());
SELECT MAX(current_time) - MIN (current_time) as knn_time FROM metrics_udf;
--SELECT * FROM result;
