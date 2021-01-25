DROP TABLE IF EXISTS ldatapoints;
DROP TABLE IF EXISTS udatapoints;
DROP TABLE IF EXISTS result;

CREATE TABLE ldatapoints (time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION, l INTEGER);
CREATE TABLE udatapoints (time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION);
CREATE TABLE result (time TIMESTAMP, l INTEGER);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION DIST(a0 DOUBLE PRECISION, b0 DOUBLE PRECISION, a1 DOUBLE PRECISION, b1 DOUBLE PRECISION, a2 DOUBLE PRECISION, b2 DOUBLE PRECISION, a3 DOUBLE PRECISION, b3 DOUBLE PRECISION, a4 DOUBLE PRECISION, b4 DOUBLE PRECISION, a5 DOUBLE PRECISION, b5 DOUBLE PRECISION, a6 DOUBLE PRECISION, b6 DOUBLE PRECISION, a7 DOUBLE PRECISION, b7 DOUBLE PRECISION, a8 DOUBLE PRECISION, b8 DOUBLE PRECISION, a9 DOUBLE PRECISION, b9 DOUBLE PRECISION, a10 DOUBLE PRECISION, b10 DOUBLE PRECISION, a11 DOUBLE PRECISION, b11 DOUBLE PRECISION, a12 DOUBLE PRECISION, b12 DOUBLE PRECISION, a13 DOUBLE PRECISION, b13 DOUBLE PRECISION, a14 DOUBLE PRECISION, b14 DOUBLE PRECISION, a15 DOUBLE PRECISION, b15 DOUBLE PRECISION, a16 DOUBLE PRECISION, b16 DOUBLE PRECISION, a17 DOUBLE PRECISION, b17 DOUBLE PRECISION, a18 DOUBLE PRECISION, b18 DOUBLE PRECISION, a19 DOUBLE PRECISION, b19 DOUBLE PRECISION, a20 DOUBLE PRECISION, b20 DOUBLE PRECISION, a21 DOUBLE PRECISION, b21 DOUBLE PRECISION, a22 DOUBLE PRECISION, b22 DOUBLE PRECISION, a23 DOUBLE PRECISION, b23 DOUBLE PRECISION, a24 DOUBLE PRECISION, b24 DOUBLE PRECISION, a25 DOUBLE PRECISION, b25 DOUBLE PRECISION, a26 DOUBLE PRECISION, b26 DOUBLE PRECISION, a27 DOUBLE PRECISION, b27 DOUBLE PRECISION, a28 DOUBLE PRECISION, b28 DOUBLE PRECISION, a29 DOUBLE PRECISION, b29 DOUBLE PRECISION, a30 DOUBLE PRECISION, b30 DOUBLE PRECISION, a31 DOUBLE PRECISION, b31 DOUBLE PRECISION, a32 DOUBLE PRECISION, b32 DOUBLE PRECISION, a33 DOUBLE PRECISION, b33 DOUBLE PRECISION, a34 DOUBLE PRECISION, b34 DOUBLE PRECISION, a35 DOUBLE PRECISION, b35 DOUBLE PRECISION, a36 DOUBLE PRECISION, b36 DOUBLE PRECISION, a37 DOUBLE PRECISION, b37 DOUBLE PRECISION, a38 DOUBLE PRECISION, b38 DOUBLE PRECISION, a39 DOUBLE PRECISION, b39 DOUBLE PRECISION, a40 DOUBLE PRECISION, b40 DOUBLE PRECISION, a41 DOUBLE PRECISION, b41 DOUBLE PRECISION, a42 DOUBLE PRECISION, b42 DOUBLE PRECISION, a43 DOUBLE PRECISION, b43 DOUBLE PRECISION, a44 DOUBLE PRECISION, b44 DOUBLE PRECISION, a45 DOUBLE PRECISION, b45 DOUBLE PRECISION) RETURNS DOUBLE PRECISION
BEGIN
	RETURN SQRT( POWER(a0 - b0, 2) ) + SQRT( POWER(a1 - b1, 2) ) + SQRT( POWER(a2 - b2, 2) ) + SQRT( POWER(a3 - b3, 2) ) + SQRT( POWER(a4 - b4, 2) ) + SQRT( POWER(a5 - b5, 2) ) + SQRT( POWER(a6 - b6, 2) ) + SQRT( POWER(a7 - b7, 2) ) + SQRT( POWER(a8 - b8, 2) ) + SQRT( POWER(a9 - b9, 2) ) + SQRT( POWER(a10 - b10, 2) ) + SQRT( POWER(a11 - b11, 2) ) + SQRT( POWER(a12 - b12, 2) ) + SQRT( POWER(a13 - b13, 2) ) + SQRT( POWER(a14 - b14, 2) ) + SQRT( POWER(a15 - b15, 2) ) + SQRT( POWER(a16 - b16, 2) ) + SQRT( POWER(a17 - b17, 2) ) + SQRT( POWER(a18 - b18, 2) ) + SQRT( POWER(a19 - b19, 2) ) + SQRT( POWER(a20 - b20, 2) ) + SQRT( POWER(a21 - b21, 2) ) + SQRT( POWER(a22 - b22, 2) ) + SQRT( POWER(a23 - b23, 2) ) + SQRT( POWER(a24 - b24, 2) ) + SQRT( POWER(a25 - b25, 2) ) + SQRT( POWER(a26 - b26, 2) ) + SQRT( POWER(a27 - b27, 2) ) + SQRT( POWER(a28 - b28, 2) ) + SQRT( POWER(a29 - b29, 2) ) + SQRT( POWER(a30 - b30, 2) ) + SQRT( POWER(a31 - b31, 2) ) + SQRT( POWER(a32 - b32, 2) ) + SQRT( POWER(a33 - b33, 2) ) + SQRT( POWER(a34 - b34, 2) ) + SQRT( POWER(a35 - b35, 2) ) + SQRT( POWER(a36 - b36, 2) ) + SQRT( POWER(a37 - b37, 2) ) + SQRT( POWER(a38 - b38, 2) ) + SQRT( POWER(a39 - b39, 2) ) + SQRT( POWER(a40 - b40, 2) ) + SQRT( POWER(a41 - b41, 2) ) + SQRT( POWER(a42 - b42, 2) ) + SQRT( POWER(a43 - b43, 2) ) + SQRT( POWER(a44 - b44, 2) ) + SQRT( POWER(a45 - b45, 2) );
END;

CREATE OR REPLACE FUNCTION knn_alg(time TIMESTAMP, l INTEGER, dist DOUBLE PRECISION) RETURNS TABLE(time TIMESTAMP, l INTEGER)
LANGUAGE PYTHON
{
	from Queue import PriorityQueue
	
	q = {}
	for i in range(len(time)):
		current_time = time[i]
		if not(current_time in q):
			q[current_time] = PriorityQueue()
		q[current_time].put( (-dist[i], l[i]) )
		while q[current_time].qsize() > 3:
			q[current_time].get()
	
	result_time = []
	result_label = []	
	for t in q:
		a = q[t]
		l_size = a.qsize()
		l = [0] * l_size
		for i in range(l_size):
			l[i] = a.get()[1]
		result_time.append(t)
		result_label.append( max(set(l), key = l.count) )
	return [result_time, result_label]
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=10000;
SET columns=46;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;
DECLARE total_size INTEGER;

SET initial_time = get_time();
COPY INTO ldatapoints FROM '/home/gabi/Thesis-master/Datasets/alabama_weather.txt_labeled.csv' USING DELIMITERS ',','\n';
COPY INTO udatapoints FROM '/home/gabi/Thesis-master/Datasets/alabama_weather.txt_unlabeled.csv' USING DELIMITERS ',','\n';
SET final_time = get_time();

SELECT
	total_size as Total_size_bytes,
        CAST( total_size  as FLOAT) / 1024.0 / 1024.0 as Total_size_megabytes,
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
        CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;
-- Writing data *************************************************************************************

-- KNN *******************************************************************************************
DECLARE initial_time_knn FLOAT;
DECLARE final_time_knn FLOAT;

SET initial_time_knn = get_time();
INSERT INTO result 
SELECT * FROM knn_alg( 
	(SELECT udatapoints.time, ldatapoints.l,
		DIST(ldatapoints.d0, udatapoints.d0, ldatapoints.d1, udatapoints.d1, ldatapoints.d2, udatapoints.d2, ldatapoints.d3, udatapoints.d3, ldatapoints.d4, udatapoints.d4, ldatapoints.d5, udatapoints.d5, ldatapoints.d6, udatapoints.d6, ldatapoints.d7, udatapoints.d7, ldatapoints.d8, udatapoints.d8, ldatapoints.d9, udatapoints.d9, ldatapoints.d10, udatapoints.d10, ldatapoints.d11, udatapoints.d11, ldatapoints.d12, udatapoints.d12, ldatapoints.d13, udatapoints.d13, ldatapoints.d14, udatapoints.d14, ldatapoints.d15, udatapoints.d15, ldatapoints.d16, udatapoints.d16, ldatapoints.d17, udatapoints.d17, ldatapoints.d18, udatapoints.d18, ldatapoints.d19, udatapoints.d19, ldatapoints.d20, udatapoints.d20, ldatapoints.d21, udatapoints.d21, ldatapoints.d22, udatapoints.d22, ldatapoints.d23, udatapoints.d23, ldatapoints.d24, udatapoints.d24, ldatapoints.d25, udatapoints.d25, ldatapoints.d26, udatapoints.d26, ldatapoints.d27, udatapoints.d27, ldatapoints.d28, udatapoints.d28, ldatapoints.d29, udatapoints.d29, ldatapoints.d30, udatapoints.d30, ldatapoints.d31, udatapoints.d31, ldatapoints.d32, udatapoints.d32, ldatapoints.d33, udatapoints.d33, ldatapoints.d34, udatapoints.d34, ldatapoints.d35, udatapoints.d35, ldatapoints.d36, udatapoints.d36, ldatapoints.d37, udatapoints.d37, ldatapoints.d38, udatapoints.d38, ldatapoints.d39, udatapoints.d39, ldatapoints.d40, udatapoints.d40, ldatapoints.d41, udatapoints.d41, ldatapoints.d42, udatapoints.d42, ldatapoints.d43, udatapoints.d43, ldatapoints.d44, udatapoints.d44, ldatapoints.d45, udatapoints.d45)
	FROM ldatapoints, udatapoints) );
SET final_time_knn = get_time();

SELECT final_time_knn - initial_time_knn as Time_seconds;
-- KNN *******************************************************************************************

