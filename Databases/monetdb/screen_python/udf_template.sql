DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS screen;
DROP FUNCTION screen_alg;

CREATE TABLE datapoints (time TIMESTAMP, <column_types>);
CREATE TABLE screen (<column_types>);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

CREATE OR REPLACE FUNCTION get_size() RETURNS INTEGER
LANGUAGE PYTHON
{
        import os
        def get_size(start_path = '<db_dir>'):
                total_size = 0
                for dirpath, dirnames, filenames in os.walk(start_path):
                        for f in filenames:
                                fp = os.path.join(dirpath, f)
                                # skip if it is symbolic link
                                if not os.path.islink(fp):
                                        total_size += os.path.getsize(fp)
                return total_size
        return get_size()
};

CREATE OR REPLACE FUNCTION screen_alg(time TIMESTAMP, <column_types>) RETURNS TABLE(<column_types>)
LANGUAGE PYTHON
{
        import sys
        import numpy as np
        sys.path.append('<implementation_path>')
        import screen
        import datetime

        matrix = np.array([<column_names>]).T
        time = [( datetime.datetime.strptime(x, '%Y-%m-%d %H:%M:%S.%f') - datetime.datetime(1970, 1, 1)).total_seconds() for x in time]
        time = [int(x) for x in time]
        results = screen.screen(matrix, time, 0.1, -0.1, 5).T

        return results
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

SET initial_size =  get_size();
SET initial_time = get_time();
COPY INTO datapoints FROM '<data_file>' USING DELIMITERS ',','\n';
SET final_time = get_time();
SET final_size = get_size();

SELECT
        final_size - initial_size as Total_size_bytes,
        CAST( (final_size - initial_size) as FLOAT) / 1024.0 / 1024.0 as Total_size_megabytes,
        final_time - initial_time as Total_time_seconds,
        CAST(lines as FLOAT) / (final_time - initial_time) AS Throughput_inserts_per_second,
        CAST(lines * columns as FLOAT) / (final_time - initial_time) AS Throughput_values_per_second;
-- Writing data *************************************************************************************


-- Screen *******************************************************************************************
DECLARE initial_time_screen FLOAT;
DECLARE final_time_screen FLOAT;

SET initial_time_screen = get_time();
INSERT INTO screen SELECT * FROM screen_alg( (SELECT * FROM datapoints) );
SET final_time_screen = get_time();

SELECT final_time_screen - initial_time_screen as Time_seconds;
-- Screen *******************************************************************************************