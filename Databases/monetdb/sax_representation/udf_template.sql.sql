DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS representation;
DROP FUNCTION saxrep_alg;

CREATE TABLE datapoints (time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION);
CREATE TABLE representation(sax TEXT);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION saxrep_alg(time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION) RETURNS TABLE(sax TEXT)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('/home/gabi/Thesis-master/Algorithms/sax')
        import saxtransformation

        matrix = np.array([d0, d1]).T
        saxrep = saxtransformation.saxrepresentation( matrix )

        return saxrep
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=10;
SET columns=2;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;
DECLARE total_size INTEGER;

SET initial_time = get_time();
COPY INTO datapoints FROM '/home/gabi/Thesis-master/Datasets/alabama_weather.txt.csv' USING DELIMITERS ',','\n';
SET final_time = get_time();
SET total_size = ( SELECT SUM(columnsize) FROM storage() WHERE table='datapoints' );

SELECT
        total_size as Total_size_bytes,
        CAST( (total_size) as FLOAT) / 1024.0 / 1024.0 as Total_size_megabytes,
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
        CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;
-- Writing data *************************************************************************************


-- SAX*******************************************************************************************
DECLARE initial_time_sax_transformation FLOAT;
DECLARE final_time_sax_transformation FLOAT;

SET initial_time_sax_transformation = get_time();
INSERT INTO representation SELECT * FROM saxrep_alg( (SELECT * FROM datapoints) );
SET final_time_sax_transformation = get_time();

SELECT final_time_sax_transformation - initial_time_sax_transformation as Time_seconds__sax;
-- SAX *******************************************************************************************

SELECT * FROM representation;
