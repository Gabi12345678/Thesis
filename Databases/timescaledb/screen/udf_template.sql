CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP FUNCTION IF EXISTS screen;
DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;
DROP FUNCTION IF EXISTS get_size;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, <column_types> );
CREATE TYPE result_type AS ( <column_types> );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE screen OF result_type;
SELECT create_hypertable('datapoints', 'time');

CREATE OR REPLACE FUNCTION get_size() RETURNS DOUBLE PRECISION AS $$
        import os
        def get_size(start_path = '/var/lib/postgresql/11/main'):
                total_size = 0
                for dirpath, dirnames, filenames in os.walk(start_path):
                        for f in filenames:
                                fp = os.path.join(dirpath, f)
                                # skip if it is symbolic link
                                if not os.path.islink(fp):
                                        total_size += os.path.getsize(fp)
                return total_size
        return get_size()
$$ LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION screen() RETURNS SETOF result_type AS $$
	import sys
	import numpy as np
	sys.path.append('<implementation_path>/')
	import screen
	from datetime import datetime
	
	a = plpy.execute("SELECT * FROM datapoints;")

	lines = <lines>
	columns = <columns>
	
	matrix = []
	timestamps = []
	for i in range(lines):
		current_row = []
		timestamps.append(a[i]['time'])
		for j in range(columns):
			current_row.append( a[i]['d' + str(j)] )
		current_row = np.array(current_row)
		matrix.append( current_row )
	timestamps = [ int( (datetime.strptime(x, '%Y-%m-%d %H:%M:%S') - datetime(1970, 1, 1)).total_seconds() ) for x in timestamps]
	matrix = np.array( matrix )
	result = screen.screen(matrix, timestamps, 0.01, -0.01, 5)
	

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
	RAISE NOTICE 'Total size megabytes = %', (final_size - initial_size) / 1024 / 1024;
	RAISE NOTICE 'Total time seconds = %', delta;
	RAISE NOTICE 'Throughput inserts per second = %', <lines> / delta;
	RAISE NOTICE 'Throughput values per second = %', <lines> * <columns> / delta;
END;
$load_data$;

DO $screen$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO screen SELECT * FROM screen();
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);
	RAISE NOTICE 'Screen time seconds = %', delta;
END;
$screen$;

SELECT * FROM datapoints;
SELECT * FROM screen;
