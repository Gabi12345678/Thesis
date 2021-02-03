CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_sum_type CASCADE;
DROP TYPE IF EXISTS result_interval_type CASCADE;
DROP TYPE IF EXISTS result_moving_type CASCADE;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION );
CREATE TYPE result_sum_type AS (s DOUBLE PRECISION);
CREATE TYPE result_interval_type AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION );
CREATE TYPE result_moving_type AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE result_sum_table of result_sum_type;
CREATE TABLE result_interval_table of result_interval_type;
CREATE TABLE result_moving_table of result_moving_type;
SELECT create_hypertable('datapoints', 'time');

DO $load_data$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	COPY datapoints FROM '/home/gabi/Thesis-master/Datasets/synthetic.txt.csv' DELIMITER ',' CSV;
	end_time := clock_timestamp();
	delta:= extract(epoch from end_time) - extract(epoch from start_time) ;
	
	RAISE NOTICE 'Total time seconds = %', delta;
	RAISE NOTICE 'Throughput values per second = %', 100 / delta;
END;
$load_data$;

DO $SIMPLESUM$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO result_sum_table SELECT SUM(d) FROM datapoints;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'SUM time seconds = %', delta;
END;
$SIMPLESUM$;

SELECT * from result_sum_table;

DO $SIMPLEINTERVAL$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO result_interval_table SELECT time, d FROM datapoints WHERE time > '2021-01-31'::timestamp AND time < '2021-02-01'::timestamp;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'INTERVAL time seconds = %', delta;
END;
$SIMPLEINTERVAL$;

DO $SIMPLEMOVINGAVERAGE$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO result_moving_table SELECT time, AVG(d) OVER (ORDER BY time ASC RANGE BETWEEN INTERVAL '24 hours'::interval PRECEDING AND INTERVAL '0'::interval FOLLOWING) FROM datapoints;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'MOVING-AVERAGE time seconds = %', delta;
END;
$SIMPLEMOVINGAVERAGE$;

