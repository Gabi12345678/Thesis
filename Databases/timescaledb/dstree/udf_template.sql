CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP FUNCTION IF EXISTS dstree;
DROP TYPE IF EXISTS datapoint CASCADE;
DROP TABLE IF EXISTS distances;
DROP FUNCTION IF EXISTS get_size;
DROP FUNCTION IF EXISTS index_builder;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE distances(dist DOUBLE PRECISION);
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

CREATE OR REPLACE FUNCTION index_builder() RETURNS TEXT AS $$
	import sys
	import numpy as np
	sys.path.append('<implementation_path>')
	from dstree.dynamicsplit import IndexBuilder

	IndexBuilder.buildIndex('<data_file>', '<index_path>', 100, 1, 1000, <columns>)
	return "Done"
$$ LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION remove_index() RETURNS TEXT AS $$
	import os
	os.system("rm -r <index_path>*")
	return "Index removed"
$$ LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION udf() RETURNS SETOF DOUBLE PRECISION AS $$
	import sys
	import numpy as np
	sys.path.append('<implementation_path>/')
	from dstree.dynamicsplit import IndexExactSearcher

	a = plpy.execute("SELECT * FROM datapoints;")

	lines = <lines>
	columns = <columns>

	matrix = []
	for i in range(lines):
		matrix.append( a[i]['d'] )
	matrix = np.array( matrix ).T

	result = IndexExactSearcher.search(matrix, '<index_path>.idx_dyn_100_1_<columns>')

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
	COPY datapoints FROM '<query_file>' DELIMITER ',' CSV;
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

DO $index_builder$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	PERFORM index_builder();
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'Index time seconds = %', delta;
END;
$index_builder$;

DO $dstree$
DECLARE
        start_time TIMESTAMP WITH TIME ZONE;
        end_time TIMESTAMP WITH TIME ZONE;
        delta DOUBLE PRECISION;
BEGIN
        start_time := clock_timestamp();
        INSERT INTO distances SELECT * FROM udf();
        end_time := clock_timestamp();
        delta = extract(epoch from end_time) - extract(epoch from start_time);

        RAISE NOTICE 'Search time seconds = %', delta;
END;
$dstree$;

select remove_index();

select * from distances limit 10;
