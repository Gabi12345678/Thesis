DROP TABLE IF EXISTS datapoints;
DROP TABLE IF EXISTS zscore;
DROP FUNCTION zscore_alg;

CREATE TABLE datapoints (time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION, d46 DOUBLE PRECISION, d47 DOUBLE PRECISION, d48 DOUBLE PRECISION, d49 DOUBLE PRECISION, d50 DOUBLE PRECISION, d51 DOUBLE PRECISION, d52 DOUBLE PRECISION, d53 DOUBLE PRECISION, d54 DOUBLE PRECISION, d55 DOUBLE PRECISION, d56 DOUBLE PRECISION, d57 DOUBLE PRECISION, d58 DOUBLE PRECISION, d59 DOUBLE PRECISION, d60 DOUBLE PRECISION, d61 DOUBLE PRECISION, d62 DOUBLE PRECISION, d63 DOUBLE PRECISION, d64 DOUBLE PRECISION, d65 DOUBLE PRECISION, d66 DOUBLE PRECISION, d67 DOUBLE PRECISION, d68 DOUBLE PRECISION, d69 DOUBLE PRECISION, d70 DOUBLE PRECISION, d71 DOUBLE PRECISION, d72 DOUBLE PRECISION, d73 DOUBLE PRECISION, d74 DOUBLE PRECISION, d75 DOUBLE PRECISION, d76 DOUBLE PRECISION, d77 DOUBLE PRECISION, d78 DOUBLE PRECISION, d79 DOUBLE PRECISION, d80 DOUBLE PRECISION, d81 DOUBLE PRECISION, d82 DOUBLE PRECISION, d83 DOUBLE PRECISION, d84 DOUBLE PRECISION, d85 DOUBLE PRECISION, d86 DOUBLE PRECISION, d87 DOUBLE PRECISION, d88 DOUBLE PRECISION, d89 DOUBLE PRECISION, d90 DOUBLE PRECISION, d91 DOUBLE PRECISION, d92 DOUBLE PRECISION, d93 DOUBLE PRECISION, d94 DOUBLE PRECISION, d95 DOUBLE PRECISION, d96 DOUBLE PRECISION, d97 DOUBLE PRECISION, d98 DOUBLE PRECISION, d99 DOUBLE PRECISION);
CREATE TABLE zscore (time TIMESTAMP, d0 DOUBLE PRECISION, d1 DOUBLE PRECISION, d2 DOUBLE PRECISION, d3 DOUBLE PRECISION, d4 DOUBLE PRECISION, d5 DOUBLE PRECISION, d6 DOUBLE PRECISION, d7 DOUBLE PRECISION, d8 DOUBLE PRECISION, d9 DOUBLE PRECISION, d10 DOUBLE PRECISION, d11 DOUBLE PRECISION, d12 DOUBLE PRECISION, d13 DOUBLE PRECISION, d14 DOUBLE PRECISION, d15 DOUBLE PRECISION, d16 DOUBLE PRECISION, d17 DOUBLE PRECISION, d18 DOUBLE PRECISION, d19 DOUBLE PRECISION, d20 DOUBLE PRECISION, d21 DOUBLE PRECISION, d22 DOUBLE PRECISION, d23 DOUBLE PRECISION, d24 DOUBLE PRECISION, d25 DOUBLE PRECISION, d26 DOUBLE PRECISION, d27 DOUBLE PRECISION, d28 DOUBLE PRECISION, d29 DOUBLE PRECISION, d30 DOUBLE PRECISION, d31 DOUBLE PRECISION, d32 DOUBLE PRECISION, d33 DOUBLE PRECISION, d34 DOUBLE PRECISION, d35 DOUBLE PRECISION, d36 DOUBLE PRECISION, d37 DOUBLE PRECISION, d38 DOUBLE PRECISION, d39 DOUBLE PRECISION, d40 DOUBLE PRECISION, d41 DOUBLE PRECISION, d42 DOUBLE PRECISION, d43 DOUBLE PRECISION, d44 DOUBLE PRECISION, d45 DOUBLE PRECISION, d46 DOUBLE PRECISION, d47 DOUBLE PRECISION, d48 DOUBLE PRECISION, d49 DOUBLE PRECISION, d50 DOUBLE PRECISION, d51 DOUBLE PRECISION, d52 DOUBLE PRECISION, d53 DOUBLE PRECISION, d54 DOUBLE PRECISION, d55 DOUBLE PRECISION, d56 DOUBLE PRECISION, d57 DOUBLE PRECISION, d58 DOUBLE PRECISION, d59 DOUBLE PRECISION, d60 DOUBLE PRECISION, d61 DOUBLE PRECISION, d62 DOUBLE PRECISION, d63 DOUBLE PRECISION, d64 DOUBLE PRECISION, d65 DOUBLE PRECISION, d66 DOUBLE PRECISION, d67 DOUBLE PRECISION, d68 DOUBLE PRECISION, d69 DOUBLE PRECISION, d70 DOUBLE PRECISION, d71 DOUBLE PRECISION, d72 DOUBLE PRECISION, d73 DOUBLE PRECISION, d74 DOUBLE PRECISION, d75 DOUBLE PRECISION, d76 DOUBLE PRECISION, d77 DOUBLE PRECISION, d78 DOUBLE PRECISION, d79 DOUBLE PRECISION, d80 DOUBLE PRECISION, d81 DOUBLE PRECISION, d82 DOUBLE PRECISION, d83 DOUBLE PRECISION, d84 DOUBLE PRECISION, d85 DOUBLE PRECISION, d86 DOUBLE PRECISION, d87 DOUBLE PRECISION, d88 DOUBLE PRECISION, d89 DOUBLE PRECISION, d90 DOUBLE PRECISION, d91 DOUBLE PRECISION, d92 DOUBLE PRECISION, d93 DOUBLE PRECISION, d94 DOUBLE PRECISION, d95 DOUBLE PRECISION, d96 DOUBLE PRECISION, d97 DOUBLE PRECISION, d98 DOUBLE PRECISION, d99 DOUBLE PRECISION);

CREATE OR REPLACE FUNCTION get_time() RETURNS FLOAT
LANGUAGE PYTHON
{
        from datetime import datetime
        return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
};

DECLARE lines INTEGER;
DECLARE columns INTEGER;
SET lines=100;
SET columns=100;

-- Writing data *************************************************************************************
DECLARE initial_time FLOAT;
DECLARE final_time FLOAT;

SET initial_time = get_time();
COPY INTO datapoints FROM '/home/gabi/Thesis-master/Datasets/synthetic.txt.csv' USING DELIMITERS ',','\n';
SET final_time = get_time();
-- Writing data *************************************************************************************


-- ZSCORE *******************************************************************************************
DECLARE initial_time_zscore FLOAT;
DECLARE final_time_zscore FLOAT;

SET initial_time_zscore = get_time();
INSERT INTO zscore SELECT time,
	(d0 - (SELECT avg(d0) FROM datapoints)) / (SELECT sys.stddev_pop(d0) FROM datapoints),
	(d1 - (SELECT avg(d1) FROM datapoints)) / (SELECT sys.stddev_pop(d1) FROM datapoints),
	(d2 - (SELECT avg(d2) FROM datapoints)) / (SELECT sys.stddev_pop(d2) FROM datapoints),
	(d3 - (SELECT avg(d3) FROM datapoints)) / (SELECT sys.stddev_pop(d3) FROM datapoints),
	(d4 - (SELECT avg(d4) FROM datapoints)) / (SELECT sys.stddev_pop(d4) FROM datapoints),
	(d5 - (SELECT avg(d5) FROM datapoints)) / (SELECT sys.stddev_pop(d5) FROM datapoints),
	(d6 - (SELECT avg(d6) FROM datapoints)) / (SELECT sys.stddev_pop(d6) FROM datapoints),
	(d7 - (SELECT avg(d7) FROM datapoints)) / (SELECT sys.stddev_pop(d7) FROM datapoints),
	(d8 - (SELECT avg(d8) FROM datapoints)) / (SELECT sys.stddev_pop(d8) FROM datapoints),
	(d9 - (SELECT avg(d9) FROM datapoints)) / (SELECT sys.stddev_pop(d9) FROM datapoints),
	(d10 - (SELECT avg(d10) FROM datapoints)) / (SELECT sys.stddev_pop(d10) FROM datapoints),
	(d11 - (SELECT avg(d11) FROM datapoints)) / (SELECT sys.stddev_pop(d11) FROM datapoints),
	(d12 - (SELECT avg(d12) FROM datapoints)) / (SELECT sys.stddev_pop(d12) FROM datapoints),
	(d13 - (SELECT avg(d13) FROM datapoints)) / (SELECT sys.stddev_pop(d13) FROM datapoints),
	(d14 - (SELECT avg(d14) FROM datapoints)) / (SELECT sys.stddev_pop(d14) FROM datapoints),
	(d15 - (SELECT avg(d15) FROM datapoints)) / (SELECT sys.stddev_pop(d15) FROM datapoints),
	(d16 - (SELECT avg(d16) FROM datapoints)) / (SELECT sys.stddev_pop(d16) FROM datapoints),
	(d17 - (SELECT avg(d17) FROM datapoints)) / (SELECT sys.stddev_pop(d17) FROM datapoints),
	(d18 - (SELECT avg(d18) FROM datapoints)) / (SELECT sys.stddev_pop(d18) FROM datapoints),
	(d19 - (SELECT avg(d19) FROM datapoints)) / (SELECT sys.stddev_pop(d19) FROM datapoints),
	(d20 - (SELECT avg(d20) FROM datapoints)) / (SELECT sys.stddev_pop(d20) FROM datapoints),
	(d21 - (SELECT avg(d21) FROM datapoints)) / (SELECT sys.stddev_pop(d21) FROM datapoints),
	(d22 - (SELECT avg(d22) FROM datapoints)) / (SELECT sys.stddev_pop(d22) FROM datapoints),
	(d23 - (SELECT avg(d23) FROM datapoints)) / (SELECT sys.stddev_pop(d23) FROM datapoints),
	(d24 - (SELECT avg(d24) FROM datapoints)) / (SELECT sys.stddev_pop(d24) FROM datapoints),
	(d25 - (SELECT avg(d25) FROM datapoints)) / (SELECT sys.stddev_pop(d25) FROM datapoints),
	(d26 - (SELECT avg(d26) FROM datapoints)) / (SELECT sys.stddev_pop(d26) FROM datapoints),
	(d27 - (SELECT avg(d27) FROM datapoints)) / (SELECT sys.stddev_pop(d27) FROM datapoints),
	(d28 - (SELECT avg(d28) FROM datapoints)) / (SELECT sys.stddev_pop(d28) FROM datapoints),
	(d29 - (SELECT avg(d29) FROM datapoints)) / (SELECT sys.stddev_pop(d29) FROM datapoints),
	(d30 - (SELECT avg(d30) FROM datapoints)) / (SELECT sys.stddev_pop(d30) FROM datapoints),
	(d31 - (SELECT avg(d31) FROM datapoints)) / (SELECT sys.stddev_pop(d31) FROM datapoints),
	(d32 - (SELECT avg(d32) FROM datapoints)) / (SELECT sys.stddev_pop(d32) FROM datapoints),
	(d33 - (SELECT avg(d33) FROM datapoints)) / (SELECT sys.stddev_pop(d33) FROM datapoints),
	(d34 - (SELECT avg(d34) FROM datapoints)) / (SELECT sys.stddev_pop(d34) FROM datapoints),
	(d35 - (SELECT avg(d35) FROM datapoints)) / (SELECT sys.stddev_pop(d35) FROM datapoints),
	(d36 - (SELECT avg(d36) FROM datapoints)) / (SELECT sys.stddev_pop(d36) FROM datapoints),
	(d37 - (SELECT avg(d37) FROM datapoints)) / (SELECT sys.stddev_pop(d37) FROM datapoints),
	(d38 - (SELECT avg(d38) FROM datapoints)) / (SELECT sys.stddev_pop(d38) FROM datapoints),
	(d39 - (SELECT avg(d39) FROM datapoints)) / (SELECT sys.stddev_pop(d39) FROM datapoints),
	(d40 - (SELECT avg(d40) FROM datapoints)) / (SELECT sys.stddev_pop(d40) FROM datapoints),
	(d41 - (SELECT avg(d41) FROM datapoints)) / (SELECT sys.stddev_pop(d41) FROM datapoints),
	(d42 - (SELECT avg(d42) FROM datapoints)) / (SELECT sys.stddev_pop(d42) FROM datapoints),
	(d43 - (SELECT avg(d43) FROM datapoints)) / (SELECT sys.stddev_pop(d43) FROM datapoints),
	(d44 - (SELECT avg(d44) FROM datapoints)) / (SELECT sys.stddev_pop(d44) FROM datapoints),
	(d45 - (SELECT avg(d45) FROM datapoints)) / (SELECT sys.stddev_pop(d45) FROM datapoints),
	(d46 - (SELECT avg(d46) FROM datapoints)) / (SELECT sys.stddev_pop(d46) FROM datapoints),
	(d47 - (SELECT avg(d47) FROM datapoints)) / (SELECT sys.stddev_pop(d47) FROM datapoints),
	(d48 - (SELECT avg(d48) FROM datapoints)) / (SELECT sys.stddev_pop(d48) FROM datapoints),
	(d49 - (SELECT avg(d49) FROM datapoints)) / (SELECT sys.stddev_pop(d49) FROM datapoints),
	(d50 - (SELECT avg(d50) FROM datapoints)) / (SELECT sys.stddev_pop(d50) FROM datapoints),
	(d51 - (SELECT avg(d51) FROM datapoints)) / (SELECT sys.stddev_pop(d51) FROM datapoints),
	(d52 - (SELECT avg(d52) FROM datapoints)) / (SELECT sys.stddev_pop(d52) FROM datapoints),
	(d53 - (SELECT avg(d53) FROM datapoints)) / (SELECT sys.stddev_pop(d53) FROM datapoints),
	(d54 - (SELECT avg(d54) FROM datapoints)) / (SELECT sys.stddev_pop(d54) FROM datapoints),
	(d55 - (SELECT avg(d55) FROM datapoints)) / (SELECT sys.stddev_pop(d55) FROM datapoints),
	(d56 - (SELECT avg(d56) FROM datapoints)) / (SELECT sys.stddev_pop(d56) FROM datapoints),
	(d57 - (SELECT avg(d57) FROM datapoints)) / (SELECT sys.stddev_pop(d57) FROM datapoints),
	(d58 - (SELECT avg(d58) FROM datapoints)) / (SELECT sys.stddev_pop(d58) FROM datapoints),
	(d59 - (SELECT avg(d59) FROM datapoints)) / (SELECT sys.stddev_pop(d59) FROM datapoints),
	(d60 - (SELECT avg(d60) FROM datapoints)) / (SELECT sys.stddev_pop(d60) FROM datapoints),
	(d61 - (SELECT avg(d61) FROM datapoints)) / (SELECT sys.stddev_pop(d61) FROM datapoints),
	(d62 - (SELECT avg(d62) FROM datapoints)) / (SELECT sys.stddev_pop(d62) FROM datapoints),
	(d63 - (SELECT avg(d63) FROM datapoints)) / (SELECT sys.stddev_pop(d63) FROM datapoints),
	(d64 - (SELECT avg(d64) FROM datapoints)) / (SELECT sys.stddev_pop(d64) FROM datapoints),
	(d65 - (SELECT avg(d65) FROM datapoints)) / (SELECT sys.stddev_pop(d65) FROM datapoints),
	(d66 - (SELECT avg(d66) FROM datapoints)) / (SELECT sys.stddev_pop(d66) FROM datapoints),
	(d67 - (SELECT avg(d67) FROM datapoints)) / (SELECT sys.stddev_pop(d67) FROM datapoints),
	(d68 - (SELECT avg(d68) FROM datapoints)) / (SELECT sys.stddev_pop(d68) FROM datapoints),
	(d69 - (SELECT avg(d69) FROM datapoints)) / (SELECT sys.stddev_pop(d69) FROM datapoints),
	(d70 - (SELECT avg(d70) FROM datapoints)) / (SELECT sys.stddev_pop(d70) FROM datapoints),
	(d71 - (SELECT avg(d71) FROM datapoints)) / (SELECT sys.stddev_pop(d71) FROM datapoints),
	(d72 - (SELECT avg(d72) FROM datapoints)) / (SELECT sys.stddev_pop(d72) FROM datapoints),
	(d73 - (SELECT avg(d73) FROM datapoints)) / (SELECT sys.stddev_pop(d73) FROM datapoints),
	(d74 - (SELECT avg(d74) FROM datapoints)) / (SELECT sys.stddev_pop(d74) FROM datapoints),
	(d75 - (SELECT avg(d75) FROM datapoints)) / (SELECT sys.stddev_pop(d75) FROM datapoints),
	(d76 - (SELECT avg(d76) FROM datapoints)) / (SELECT sys.stddev_pop(d76) FROM datapoints),
	(d77 - (SELECT avg(d77) FROM datapoints)) / (SELECT sys.stddev_pop(d77) FROM datapoints),
	(d78 - (SELECT avg(d78) FROM datapoints)) / (SELECT sys.stddev_pop(d78) FROM datapoints),
	(d79 - (SELECT avg(d79) FROM datapoints)) / (SELECT sys.stddev_pop(d79) FROM datapoints),
	(d80 - (SELECT avg(d80) FROM datapoints)) / (SELECT sys.stddev_pop(d80) FROM datapoints),
	(d81 - (SELECT avg(d81) FROM datapoints)) / (SELECT sys.stddev_pop(d81) FROM datapoints),
	(d82 - (SELECT avg(d82) FROM datapoints)) / (SELECT sys.stddev_pop(d82) FROM datapoints),
	(d83 - (SELECT avg(d83) FROM datapoints)) / (SELECT sys.stddev_pop(d83) FROM datapoints),
	(d84 - (SELECT avg(d84) FROM datapoints)) / (SELECT sys.stddev_pop(d84) FROM datapoints),
	(d85 - (SELECT avg(d85) FROM datapoints)) / (SELECT sys.stddev_pop(d85) FROM datapoints),
	(d86 - (SELECT avg(d86) FROM datapoints)) / (SELECT sys.stddev_pop(d86) FROM datapoints),
	(d87 - (SELECT avg(d87) FROM datapoints)) / (SELECT sys.stddev_pop(d87) FROM datapoints),
	(d88 - (SELECT avg(d88) FROM datapoints)) / (SELECT sys.stddev_pop(d88) FROM datapoints),
	(d89 - (SELECT avg(d89) FROM datapoints)) / (SELECT sys.stddev_pop(d89) FROM datapoints),
	(d90 - (SELECT avg(d90) FROM datapoints)) / (SELECT sys.stddev_pop(d90) FROM datapoints),
	(d91 - (SELECT avg(d91) FROM datapoints)) / (SELECT sys.stddev_pop(d91) FROM datapoints),
	(d92 - (SELECT avg(d92) FROM datapoints)) / (SELECT sys.stddev_pop(d92) FROM datapoints),
	(d93 - (SELECT avg(d93) FROM datapoints)) / (SELECT sys.stddev_pop(d93) FROM datapoints),
	(d94 - (SELECT avg(d94) FROM datapoints)) / (SELECT sys.stddev_pop(d94) FROM datapoints),
	(d95 - (SELECT avg(d95) FROM datapoints)) / (SELECT sys.stddev_pop(d95) FROM datapoints),
	(d96 - (SELECT avg(d96) FROM datapoints)) / (SELECT sys.stddev_pop(d96) FROM datapoints),
	(d97 - (SELECT avg(d97) FROM datapoints)) / (SELECT sys.stddev_pop(d97) FROM datapoints),
	(d98 - (SELECT avg(d98) FROM datapoints)) / (SELECT sys.stddev_pop(d98) FROM datapoints),
	(d99 - (SELECT avg(d99) FROM datapoints)) / (SELECT sys.stddev_pop(d99) FROM datapoints)
FROM datapoints;
SET final_time_zscore = get_time();

SELECT final_time_zscore - initial_time_zscore as Time_seconds;
-- ZSCORE *******************************************************************************************
