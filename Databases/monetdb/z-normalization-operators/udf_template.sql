DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS zscore;
DROP FUNCTION zscore_alg;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE zscore (time TIMESTAMP, <column_types>);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=<lines>;
SET columns=<columns>;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;

SET initial_time = get_time();
COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n';
SET final_time = get_time();
-- Writing data *************************************************************************************


-- ZSCORE *******************************************************************************************
DECLARE initial_time_zscore FLOAT;
DECLARE final_time_zscore FLOAT;

SET initial_time_zscore = get_time();
INSERT INTO zscore SELECT time,
<select_expression>
FROM datapoints;
SET final_time_zscore = get_time();

SELECT final_time_zscore - initial_time_zscore as Time_seconds;
-- ZSCORE *******************************************************************************************
