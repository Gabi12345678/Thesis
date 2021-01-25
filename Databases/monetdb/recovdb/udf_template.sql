DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS missing_values;
DROP FUNCTION mv_alg;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE missing_values (<column_types>);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
	from datetime import datetime
	return (datetime.now() - datetime(1970, 1, 1)).total_seconds();
};


CREATE OR REPLACE FUNCTION mv_alg(time TIMESTAMP, <column_types>) RETURNS TABLE(<column_types>)
LANGUAGE PYTHON
{
	import sys
	import numpy as np
	sys.path.append("<implementation_path>")
	import recovery

	matrix = np.array([<column_names>]).T
	n = matrix.shape[0]
	m = matrix.shape[1]
	k = 3
	rec_time, iter, rmse, rec_mat = recovery.recovery(matrix, n, m, k, 0.2, 10)
	return rec_mat.T.tolist()
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=<lines>;
SET columns=<columns>;

DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;
DECLARE total_size INTEGER;

SET initial_time = get_time();
COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n' NULL AS '';
SET final_time = get_time();
SET total_size = (SELECT SUM(columnsize) FROM storage() WHERE table='datapoints');

SELECT
	total_size as Total_size_bytes,
	CAST( total_size as FLOAT) / 1024.0 / 1024.0 as Total_size_megabytes,
	final_time - initial_time as Total_time_seconds,
	CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
	CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;


DECLARE initial_time_recov FLOAT;
DECLARE final_time_recov FLOAT;

SET initial_time_recov = get_time();
INSERT INTO missing_values SELECT * FROM mv_alg( (SELECT * FROM datapoints) );
SET final_time_recov = get_time();

SELECT final_time_recov - initial_time_recov as Time_seconds;

