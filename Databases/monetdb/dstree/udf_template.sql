DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS distances;
DROP FUNCTION IF EXISTS index_builder;
DROP FUNCTION IF EXISTS udf;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE distances (dist DOUBLE PRECISION);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};


CREATE OR REPLACE FUNCTION index_builder() RETURNS VARCHAR(10)
LANGUAGE PYTHON
{
	import sys
	sys.path.append('<implementation_path>')
	from dstree.dynamicsplit import IndexBuilder

	IndexBuilder.buildIndex("<data_file>", "<index_path>", 100, 1, 1000, <columns>)
	return "Done"
};

CREATE OR REPLACE FUNCTION udf(time TIMESTAMP, <column_types>) RETURNS TABLE(dist DOUBLE PRECISION)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('<implementation_path>')
        import dstree.dynamicsplit
        import dstree.util
        from dstree.dynamicsplit import *
        reload(dstree.dynamicsplit)
        reload(dstree.util)

        matrix = np.array([<column_names>])
        distances = IndexExactSearcher.search(matrix, "<index_path>.idx_dyn_100_1_<columns>")

        return distances
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=<lines>;
SET columns=<columns>;

-- Writing data *************************************************************************************
DECLARE initial_size INTEGER;
DECLARE final_size INTEGER;
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;

SET initial_time = get_time();
COPY INTO datapoints FROM '<query_file>' USING DELIMITERS ',','\n';
SET final_time = get_time();

SELECT
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
        CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;
-- Writing data *************************************************************************************

-- Index ********************************************************************************************
DECLARE initial_time_index FLOAT;
DECLARE final_time_index FLOAT;

SET initial_time_index = get_time();
SELECT index_builder();
SET final_time_index = get_time();

SELECT final_time_index - initial_time_index as Index_time_seconds;
-- Index ********************************************************************************************

-- Search *******************************************************************************************
DECLARE initial_time_search FLOAT;
DECLARE final_time_search FLOAT;

SET initial_time_search = get_time();
INSERT INTO distances SELECT * FROM udf( (SELECT * FROM datapoints) );
SET final_time_search = get_time();

SELECT final_time_search - initial_time_search as Search_time_seconds;
-- Search *******************************************************************************************
