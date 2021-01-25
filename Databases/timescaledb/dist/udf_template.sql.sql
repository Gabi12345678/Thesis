CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, a DOUBLE PRECISION ARRAY, b DOUBLE PRECISION ARRAY);
CREATE TYPE result_type AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE result of result_type;
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
	COPY datapoints FROM '/home/gabi/Thesis-master/Datasets/sport.txt.csv' DELIMITER ',' CSV;
	end_time := clock_timestamp();
END;
$load_data$;

DO $knn$
DECLARE
	start_time TIMESTAMP WITH TIME ZONE;
	end_time TIMESTAMP WITH TIME ZONE;
	delta DOUBLE PRECISION;
BEGIN
	start_time := clock_timestamp();
	INSERT INTO result SELECT time, sqrt(power(a[1]-b[1],2.0) + power(a[2]-b[2],2.0)) FROM datapoints;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'DIST time seconds = %', delta;
END;
$knn$;

--SELECT * from datapoints;
--SELECT * FROM result;
