DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result_interval;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE result_interval (time TIMESTAMP, <column_types>);

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
COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n';
-- Writing data *************************************************************************************


-- KMeans *******************************************************************************************
DECLARE initial_time_kmeans FLOAT;
DECLARE final_time_kmeans FLOAT;

SET initial_time_kmeans = get_time();
INSERT INTO result_interval SELECT * FROM datapoints WHERE time > str_to_timestamp('<start_time>', '%Y-%m-%dT%H:%M') and time < str_to_timestamp('<end_time>', '%Y-%m-%dT%H:%M');
SET final_time_kmeans = get_time();

SELECT final_time_kmeans - initial_time_kmeans as Time_seconds;
SELECT count(*) FROM result_interval;
-- KMeans *******************************************************************************************

