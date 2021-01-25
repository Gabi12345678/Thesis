DROP TABLE IF EXISTS ldatapoints;
DROP TABLE IF EXISTS udatapoints;
DROP TABLE IF EXISTS result;

CREATE TABLE ldatapoints (time TIMESTAMP, <column_types>, l INTEGER);
CREATE TABLE udatapoints (time TIMESTAMP, <column_types>);
CREATE TABLE result (time TIMESTAMP, l INTEGER);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION DIST(<dist_arg>) RETURNS DOUBLE PRECISION
BEGIN
	RETURN <distance_formula>;
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
SET lines=<lines>;
SET columns=<columns>;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;
DECLARE total_size INTEGER;

SET initial_time = get_time();
COPY INTO ldatapoints FROM '<data_file_labeled>' USING DELIMITERS ',','\n';
COPY INTO udatapoints FROM '<data_file_unlabeled>' USING DELIMITERS ',','\n';
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
		DIST(<dist_param>)
	FROM ldatapoints, udatapoints) );
SET final_time_knn = get_time();

SELECT final_time_knn - initial_time_knn as Time_seconds;
-- KNN *******************************************************************************************

