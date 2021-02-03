DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS distances;
DROP FUNCTION IF EXISTS index_builder;
DROP FUNCTION IF EXISTS udf;

CREATE TABLE datapoints (time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION, d46 DOUBLE PRECISION, d47 DOUBLE PRECISION, d48 DOUBLE PRECISION, d49 DOUBLE PRECISION);
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
	sys.path.append('/home/gabi/Thesis-master/Algorithms/dstree_python')
	from dstree.dynamicsplit import IndexBuilder

	IndexBuilder.buildIndex("/home/gabi/Thesis-master/Datasets/synthetic.txt_index.txt", "/home/gabi/Thesis-master/Databases/monetdb/dstree/out/index", 100, 1, 1000, 50)
	return "Done"
};

CREATE OR REPLACE FUNCTION udf(time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION, d46 DOUBLE PRECISION, d47 DOUBLE PRECISION, d48 DOUBLE PRECISION, d49 DOUBLE PRECISION) RETURNS TABLE(dist DOUBLE PRECISION)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('/home/gabi/Thesis-master/Algorithms/dstree_python')
        import dstree.dynamicsplit
        import dstree.util
        from dstree.dynamicsplit import *
        reload(dstree.dynamicsplit)
        reload(dstree.util)

        matrix = np.array([d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33, d34, d35, d36, d37, d38, d39, d40, d41, d42, d43, d44, d45, d46, d47, d48, d49])
        distances = IndexExactSearcher.search(matrix, "/home/gabi/Thesis-master/Databases/monetdb/dstree/out/index.idx_dyn_100_1_50")

        return distances
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=100;
SET columns=50;

-- Writing data *************************************************************************************
DECLARE initial_size INTEGER;
DECLARE final_size INTEGER;
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;

SET initial_time = get_time();
COPY INTO datapoints FROM '/home/gabi/Thesis-master/Datasets/synthetic.txt.csv' USING DELIMITERS ',','\n';
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
