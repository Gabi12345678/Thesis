-- Prepares simple table for count, approxdc
create table SimpleSequence9(
testNumber unsigned(4) primary key, 
iVal1 sequence(int(4)), 
iVal2 sequence(int(4)) );
INSERT INTO SimpleSequence9(testNumber,iVal1,iVal2)
VALUES(1,[1,1,2,2,2,3],[1,2,3,4,5,6]);

-- count 
SELECT iVal1, iVal2,
	   seq_hash_agg_count(iVal1) as count_val1,
 	   seq_hash_group_by(count_val1) as "group_by_count"
FROM SimpleSequence9 WHERE testNumber=1;

-- approxdc
SELECT iVal1, iVal2,
	   seq_hash_agg_approxdc(iVal1, iVal2) as approxdc_val1_val2,
 	   seq_hash_group_by(approxdc_val1_val2) as "group_by_approxdc"
FROM SimpleSequence9 WHERE testNumber=1;

-- _max, _min, _sum, _avg
--SELECT symbol, volume, close,
--	   seq_hash_agg_max(close, volume/10) as max_by_volume,
--	   seq_hash_agg_min(close, volume/10) as "min", 
--	   seq_hash_agg_sum(close, volume/10) as "sum",
--	   seq_hash_agg_avg(close, volume/10) as "avg", 
--	   seq_hash_group_by(max_by_volume) as "group_by"
--FROM Quote WHERE symbol='SYM0';


