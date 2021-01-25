CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY);
CREATE TYPE result_type AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE zscore OF result_type;
SELECT create_hypertable('datapoints', 'time');

DO $load_data$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
	initial_size DOUBLE PRECISION;
	final_size DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	COPY datapoints FROM '/home/gabi/Thesis-master/Datasets/hydraulic.txt.csv' DELIMITER ',' CSV;
	end_time := clock_timestamp();
END;
$load_data$;

DO $zscore$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO zscore 
	WITH 	raw_values AS (
        		SELECT time, nr, elem 
        		FROM datapoints, UNNEST(d) WITH ORDINALITY a(elem, nr)
    		),
		avg_stddev AS ( 
		        SELECT nr, avg(elem) avg, stddev_pop(elem) stddev
		        FROM raw_values GROUP BY nr 
	    	) 
		SELECT time, array_agg( 
    			( elem - (SELECT avg FROM avg_stddev WHERE avg_stddev.nr = raw_values.nr) ) / (SELECT stddev FROM avg_stddev WHERE avg_stddev.nr = raw_values.nr) 
        		ORDER BY nr ) d
		FROM raw_values GROUP BY time;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'Z-Score time seconds = %', delta;
END;
$zscore$;

--select * from datapoints;
--select * from zscore;
