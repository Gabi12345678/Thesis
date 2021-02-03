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
	COPY datapoints FROM '/home/gabi/Thesis-master/Datasets/synthetic.txt.csv' DELIMITER ',' CSV;
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
	INSERT INTO result SELECT time, sqrt(power(a[1]-b[1],2.0) + power(a[2]-b[2],2.0) + power(a[3]-b[3],2.0) + power(a[4]-b[4],2.0) + power(a[5]-b[5],2.0) + power(a[6]-b[6],2.0) + power(a[7]-b[7],2.0) + power(a[8]-b[8],2.0) + power(a[9]-b[9],2.0) + power(a[10]-b[10],2.0) + power(a[11]-b[11],2.0) + power(a[12]-b[12],2.0) + power(a[13]-b[13],2.0) + power(a[14]-b[14],2.0) + power(a[15]-b[15],2.0) + power(a[16]-b[16],2.0) + power(a[17]-b[17],2.0) + power(a[18]-b[18],2.0) + power(a[19]-b[19],2.0) + power(a[20]-b[20],2.0) + power(a[21]-b[21],2.0) + power(a[22]-b[22],2.0) + power(a[23]-b[23],2.0) + power(a[24]-b[24],2.0) + power(a[25]-b[25],2.0) + power(a[26]-b[26],2.0) + power(a[27]-b[27],2.0) + power(a[28]-b[28],2.0) + power(a[29]-b[29],2.0) + power(a[30]-b[30],2.0) + power(a[31]-b[31],2.0) + power(a[32]-b[32],2.0) + power(a[33]-b[33],2.0) + power(a[34]-b[34],2.0) + power(a[35]-b[35],2.0) + power(a[36]-b[36],2.0) + power(a[37]-b[37],2.0) + power(a[38]-b[38],2.0) + power(a[39]-b[39],2.0) + power(a[40]-b[40],2.0) + power(a[41]-b[41],2.0) + power(a[42]-b[42],2.0) + power(a[43]-b[43],2.0) + power(a[44]-b[44],2.0) + power(a[45]-b[45],2.0) + power(a[46]-b[46],2.0) + power(a[47]-b[47],2.0) + power(a[48]-b[48],2.0) + power(a[49]-b[49],2.0) + power(a[50]-b[50],2.0)) FROM datapoints;
	end_time := clock_timestamp();
	delta = extract(epoch from end_time) - extract(epoch from start_time);

	RAISE NOTICE 'DIST time seconds = %', delta;
END;
$knn$;

--SELECT * from datapoints;
--SELECT * FROM result;
