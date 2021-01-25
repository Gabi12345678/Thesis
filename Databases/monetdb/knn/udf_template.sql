DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS result;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>, l INTEGER);
CREATE TABLE result (l INTEGER);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION knn_alg(time TIMESTAMP, <column_types>, l INTEGER) RETURNS TABLE(l INTEGER)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('<implementation_path>')
        import knn

        cluster_count = 10

        matrix = np.array([<unlabel_column_names>]).T
        label_matrix = np.array([<label_column_names>]).T

        result = knn.knn(label_matrix, l, matrix, 3)

        return [result]
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
        CAST( total_size  as FLOAT) / 1024.0 / 1024.0 as Total_size_megabytes,
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
        CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;
-- Writing data *************************************************************************************


-- KNN *******************************************************************************************
DECLARE initial_time_knn FLOAT;
DECLARE final_time_knn FLOAT;

SET initial_time_knn = get_time();
INSERT INTO result SELECT * FROM knn_alg( (SELECT * FROM datapoints) );
SET final_time_knn = get_time();

SELECT final_time_knn - initial_time_knn as Time_seconds;
-- KNN *******************************************************************************************

