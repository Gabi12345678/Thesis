DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result_sum;
DROP TABLE IF EXISTS result_interval;
DROP TABLE IF EXISTS result_moving;

CREATE TABLE datapoints (time TIMESTAMP, d DOUBLE);
CREATE TABLE result_sum (s DOUBLE);
CREATE TABLE result_interval (time TIMESTAMP, s DOUBLE);
CREATE TABLE result_moving (time TIMESTAMP, d DOUBLE);
CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

DECLARE lines INTEGER;
SET lines=100;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;
DECLARE total_size INTEGER;

SET initial_time = get_time();
COPY INTO datapoints FROM '/home/gabi/Thesis-master/Datasets/synthetic.txt.csv' USING DELIMITERS ',','\n';
SET final_time = get_time();

SELECT
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second;
-- Writing data *************************************************************************************


-- SUM *****************************************************************************************
DECLARE initial_time_sum FLOAT;
DECLARE final_time_sum FLOAT;

SET initial_time_sum = get_time();
INSERT INTO result_sum SELECT SUM(d) FROM datapoints;
SET final_time_sum = get_time();

SELECT final_time_sum - initial_time_sum as time_sum;
 -- SUM ****************************************************************************************
SELECT * FROM result_table;

-- INTERVAL ************************************************************************************
DECLARE initial_time_interval FLOAT;
DECLARE final_time_interval FLOAT;

SET initial_time_interval = get_time();
INSERT INTO result_interval SELECT time, d FROM datapoints WHERE time > str_to_timestamp('2021-01-31', '%Y-%m-%d') and time < str_to_timestamp('2021-02-01', '%Y-%m-%d');
SET final_time_interval = get_time();

SELECT final_time_interval - initial_time_interval AS time_interval;
-- INTERVAL ************************************************************************************

-- MOVING AVERAGE ******************************************************************************
DECLARE initial_time_moving FLOAT;
DECLARE final_time_moving FLOAT;

SET initial_time_moving = get_time();
INSERT INTO result_moving SELECT time, AVG(d) OVER (ORDER BY time ASC RANGE BETWEEN INTERVAL '24' HOUR PRECEDING AND INTERVAL '0' HOUR FOLLOWING) FROM datapoints;
SET final_time_moving = get_time();

SELECT final_time_moving - initial_time_moving AS time_moving;
-- MOVING AVERAGE ******************************************************************************
--SELECT * FROM result_moving;
