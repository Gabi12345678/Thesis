CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_sum_type CASCADE;
DROP TYPE IF EXISTS result_interval_type CASCADE;
DROP TYPE IF EXISTS result_moving_type CASCADE;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY );
CREATE TYPE result_interval_type AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY);

CREATE TABLE datapoints OF datapoint;
CREATE TABLE result_interval_table of result_interval_type;
SELECT create_hypertable('datapoints', 'time');

DO $load_data$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	COPY datapoints FROM '<data_file>' DELIMITER ',' CSV;
	end_time := clock_timestamp();
	delta:= extract(epoch from end_time) - extract(epoch from start_time) ;
	
	RAISE NOTICE 'Total time seconds = %', delta;
	RAISE NOTICE 'Throughput values per second = %', <lines> / delta;
END;
$load_data$;

DO $SIMPLEINTERVAL$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO result_interval_table SELECT time, d FROM datapoints WHERE time > '<start_time>'::timestamp AND time < '<end_time>'::timestamp;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'INTERVAL time seconds = %', delta;
END;
$SIMPLEINTERVAL$;

select count(*) from result_interval_table;
