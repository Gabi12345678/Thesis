CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP FUNCTION IF EXISTS knn;
DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;
DROP TYPE IF EXISTS label_datapoint CASCADE;
DROP FUNCTION IF EXISTS get_size;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY);
CREATE TYPE label_datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY, label INTEGER);
CREATE TYPE result_type AS ( label INTEGER );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE label_datapoints of label_datapoint;
CREATE TABLE result of result_type;
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

CREATE OR REPLACE FUNCTION knn() RETURNS SETOF result_type AS $$
	import sys
	import numpy as np
	sys.path.append('<implementation_path>/')
	import knn
	
	a = plpy.execute("SELECT * FROM datapoints ORDER BY time ASC;")

	lines = <lines>
	columns = <columns>	
	matrix = []
	for i in range(lines):
		matrix.append( a[i]['d'] )
	matrix = np.array( matrix )

	a = plpy.execute("SELECT * FROM label_datapoints ORDER BY time ASC;")
	label_matrix = []
	labels = []
	for i in range(lines):
		label_matrix.append( a[i]['d'] )
		labels.append( a[i]['label'] )
	label_matrix = np.array(label_matrix)
	
	result = knn.knn(label_matrix, labels, matrix, 3)

	result = [ [x] for x in result ]

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
	COPY label_datapoints from '<label_data_file>' DELIMITER ',' CSV;
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

DO $knn$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO result SELECT * FROM knn();
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'KNN time seconds = %', delta;
END;
$knn$;
