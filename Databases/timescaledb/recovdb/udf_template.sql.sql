CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP FUNCTION IF EXISTS mv;
DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;
DROP FUNCTION IF EXISTS get_size;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION);
CREATE TYPE result_type AS (d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION);

CREATE TABLE datapoints OF datapoint;
CREATE TABLE missing_values OF result_type;

SELECT create_hypertable('datapoints', 'time');

CREATE OR REPLACE FUNCTION get_size() RETURNS DOUBLE PRECISION AS $$
	import os
	def get_size(start_path = '/var/lib/postgresql/11/main'):
		total_size = 0
		for dirpath, dirnames, filenames in os.walk(start_path):
			for f in filenames:
				fp = os.path.join(dirpath, f)
				if not os.path.islink(fp):
					total_size += os.path.getsize(fp)
		return total_size
	return get_size()
$$ LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION mv() RETURNS SETOF result_type AS $$
	import sys
	import numpy as np
	sys.path.append('/home/gabi/Algorithms/recovdb')
	import recovery
	
	a = plpy.execute("SELECT * FROM datapoints;")

	lines = 3500000 
	columns = 46

	matrix = []
	for i in range(lines):
		current_row = []
		for j in range(columns):
			if a[i]['d' + str(j)] == None:
				current_row.append( np.nan )
			else:
				current_row.append( a[i]['d'+str(j)] )
		current_row = np.array(current_row)
		matrix.append( current_row )
	matrix = np.array( matrix )
	
	rec_time,iterations,rmse,rec_mat = recovery.recovery(matrix, lines, columns, columns - 1, 0, 0)
	
	result = []
	for i in range(lines):
		result.append( rec_mat[i].tolist() )
	return result
$$ LANGUAGE plpythonu;

DO $load_data$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
	initial_size DOUBLE PRECISION;
	final_size DOUBLE PRECISION;
BEGIN
	initial_size := get_size();
	start_time := clock_timestamp();
	COPY datapoints FROM '/home/gabi/Datasets/real.txt.csv' DELIMITER ',' CSV;
	end_time := clock_timestamp();
	final_size := get_size();
	delta:= extract(epoch from end_time) - extract(epoch from start_time) ;

	RAISE NOTICE 'Total size bytes = %', final_size - initial_size;
	--RAISE NOTICE 'Total size megabytes = %', (final_size - initial_size) / 1024 / 1024;
	RAISE NOTICE 'Total time seconds = %', delta;
	RAISE NOTICE 'Throughput inserts per second = %', 3500000 / delta;
	RAISE NOTICE 'Throughput values per second = %', 3500000 * 46 / delta;
END;
$load_data$;

DO $mv$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO missing_values SELECT * FROM mv();
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'Recov time seconds = %', delta;
END;
$mv$;
