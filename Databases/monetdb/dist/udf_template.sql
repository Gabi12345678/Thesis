DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result;

CREATE TABLE datapoints (time TIMESTAMP, <a_column_types>, <b_column_types>);
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
COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n';
SET final_time = get_time();
-- Writing data *************************************************************************************

-- KNN *******************************************************************************************
DECLARE initial_time_knn FLOAT;
DECLARE final_time_knn FLOAT;

SET initial_time_knn = get_time();
INSERT INTO result SELECT time, SQRT(<distance_exp>) FROM datapoints;
SET final_time_knn = get_time();

SELECT final_time_knn - initial_time_knn as Time_seconds;
-- KNN *******************************************************************************************

--SELECT * FROM datapoints;
--SELECT * FROM result;
