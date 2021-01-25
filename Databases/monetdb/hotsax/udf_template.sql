DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result_anomalies;
DROP FUNCTION sax_alg;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE result_anomalies(time TIMESTAMP, index_timeseries INTEGER, value DOUBLE PRECISION);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION sax_alg(time TIMESTAMP, <column_types>) RETURNS TABLE(time TIMESTAMP, index_timeseries INTEGER, value DOUBLE PRECISION)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('<implementation_path>')
        import hotsax

        matrix = np.array([<column_names>]).T
        discord = hotsax.hotsax( matrix )

        r_time = []
        r_index = []
        r_value = []
        for i in range(len(discord)):
                r_time.append( time[ discord[i][1] ] )
                r_index.append( discord[i][0] )
                r_value.append( discord[i][2] )
        return [r_time, r_index, r_value]
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
SET total_size = ( SELECT SUM(columnsize) FROM storage() WHERE table='datapoints');

SELECT
        total_size as Total_size_bytes,
        CAST( (total_size) as FLOAT) / 1024.0 / 1024.0 as Total_size_megabytes,
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
        CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;
-- Writing data *************************************************************************************


-- SAX*******************************************************************************************
DECLARE initial_time_sax FLOAT;
DECLARE final_time_sax FLOAT;

SET initial_time_sax = get_time();
INSERT INTO result_anomalies SELECT * FROM sax_alg( (SELECT * FROM datapoints) );
SET final_time_sax = get_time();

SELECT final_time_sax - initial_time_sax as Time_seconds_anomalies_detect_sax;
-- SAX *******************************************************************************************
