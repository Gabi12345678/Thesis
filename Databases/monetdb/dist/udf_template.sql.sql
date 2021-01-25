DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result;

CREATE TABLE datapoints (time TIMESTAMP, a0 DOUBLE PRECISION, a1 DOUBLE PRECISION, b0 DOUBLE PRECISION, b1 DOUBLE PRECISION);
CREATE TABLE result (time TIMESTAMP, d DOUBLE PRECISION);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;

SET initial_time = get_time();
COPY INTO datapoints FROM '/home/gabi/Thesis-master/Datasets/sport.txt.csv' USING DELIMITERS ',','\n';
SET final_time = get_time();
-- Writing data *************************************************************************************

-- KNN *******************************************************************************************
DECLARE initial_time_knn FLOAT;
DECLARE final_time_knn FLOAT;

SET initial_time_knn = get_time();
INSERT INTO result SELECT time, SQRT(POWER(a0 - b0, 2) + POWER(a1 - b1, 2)) FROM datapoints;
SET final_time_knn = get_time();

SELECT final_time_knn - initial_time_knn as Time_seconds;
-- KNN *******************************************************************************************

--SELECT * FROM datapoints;
--SELECT * FROM result;
