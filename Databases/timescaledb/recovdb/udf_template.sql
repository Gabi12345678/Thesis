CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP FUNCTION IF EXISTS mv;
DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;
DROP FUNCTION IF EXISTS get_size;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, <column_types>);
CREATE TYPE result_type AS (<column_types>);

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
	sys.path.append('<implementation_path>')
	import recovery
	
	a = plpy.execute("SELECT * FROM datapoints;")

	lines = <lines> 
	columns = <columns>

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
	COPY datapoints FROM '<data_file>' DELIMITER ',' CSV;
	end_time := clock_timestamp();
	final_size := get_size();
	delta:= extract(epoch from end_time) - extract(epoch from start_time) ;

	RAISE NOTICE 'Total size bytes = %', final_size - initial_size;
	--RAISE NOTICE 'Total size megabytes = %', (final_size - initial_size) / 1024 / 1024;
	RAISE NOTICE 'Total time seconds = %', delta;
	RAISE NOTICE 'Throughput inserts per second = %', <lines> / delta;
	RAISE NOTICE 'Throughput values per second = %', <lines> * <columns> / delta;
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
