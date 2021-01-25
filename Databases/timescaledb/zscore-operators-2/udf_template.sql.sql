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
	SELECT time, array[
(d[1] - (select avg(d[1]) from datapoints)) / (select stddev_pop(d[1]) from datapoints),	(d[2] - (select avg(d[2]) from datapoints)) / (select stddev_pop(d[2]) from datapoints)
	]
	FROM datapoints;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'Z-Score time seconds = %', delta;
END;
$zscore$;

--select * from datapoints;
--select * from zscore;
