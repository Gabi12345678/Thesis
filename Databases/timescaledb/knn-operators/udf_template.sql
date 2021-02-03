CREATE EXTENSION IF NOT EXISTS plpythonu;
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

DROP TYPE IF EXISTS datapoint CASCADE;
DROP TYPE IF EXISTS result_type CASCADE;
DROP TYPE IF EXISTS label_datapoint CASCADE;
DROP FUNCTION IF EXISTS dist;

CREATE TYPE datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY);
CREATE TYPE label_datapoint AS ( time TIMESTAMP WITHOUT TIME ZONE, d DOUBLE PRECISION ARRAY, label INTEGER);
CREATE TYPE result_type AS ( time TIMESTAMP WITHOUT TIME ZONE, label INTEGER );

CREATE TABLE datapoints OF datapoint;
CREATE TABLE label_datapoints of label_datapoint;
CREATE TABLE result of result_type;
SELECT create_hypertable('datapoints', 'time');

CREATE OR REPLACE FUNCTION dist(a DOUBLE PRECISION ARRAY, b DOUBLE PRECISION ARRAY) RETURNS DOUBLE PRECISION AS $$
DECLARE
	l INTEGER;
	i INTEGER;
	d DOUBLE PRECISION;
BEGIN
	d := 0.0;
	l := array_length(a, 1);
	FOR i IN 1..l LOOP
		d := d + (a[i] - b[i]) * (a[i] - b[i]);
	END LOOP;
	d = sqrt(d);
	RETURN d;
END
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION knn() RETURNS SETOF result_type AS $$
	from Queue import PriorityQueue
	a = plpy.execute('SELECT datapoints.time, label_datapoints.label, dist(datapoints.d, label_datapoints.d) FROM datapoints, label_datapoints')
	q = {}
	for i in range(len(a)):
		current_time = a[i]['time']
		if not(current_time in q):
			q[current_time] = PriorityQueue()
		q[current_time].put( (-a[i]['dist'], a[i]['label']) )
		while q[current_time].qsize() > 3:
			q[current_time].get()

	result = []
	for t in q:
		a = q[t]
		l_size = a.qsize()
		l = [0] * l_size
		for i in range(l_size):
			l[i] = a.get()[1]
		result.append( [t,  max(set(l), key = l.count)] )
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
	start_time := clock_timestamp();
	COPY datapoints FROM '<data_file>' DELIMITER ',' CSV;
	end_time := clock_timestamp();
	delta:= extract(epoch from end_time) - extract(epoch from start_time) ;
	
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

--SELECT * FROM result;
