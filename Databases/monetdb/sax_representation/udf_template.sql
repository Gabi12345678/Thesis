DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS representation;
DROP FUNCTION saxrep_alg;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE representation(sax TEXT);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION saxrep_alg(time TIMESTAMP, <column_types>) RETURNS TABLE(sax TEXT)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('<implementation_path>')
        import saxtransformation

        matrix = np.array([<column_names>]).T
        saxrep = saxtransformation.saxrepresentation( matrix )

        return saxrep
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=<lines>;
SET columns=<columns>;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;
DECLARE total_size INTEGER;

SET initial_time = get_time();
COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n';
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
