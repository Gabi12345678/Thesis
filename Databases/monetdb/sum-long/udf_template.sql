DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result;

CREATE TABLE datapoints (t TIMESTAMP, <column_types>);
CREATE TABLE result (<column_types>);

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

COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n';

-- KMeans *******************************************************************************************
DECLARE initial_time_sum FLOAT;
DECLARE final_time_sum FLOAT;

SET initial_time_sum = get_time();
INSERT INTO result SELECT <sum_expr> FROM datapoints;
SET final_time_sum = get_time();

SELECT final_time_sum - initial_time_sum as Time_seconds;
-- KMeans *******************************************************************************************

