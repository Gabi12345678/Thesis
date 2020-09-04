-- ema
numformat "%6.2f"
seqformat short 6 6
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
		volume@Q1_13 as "Volume_Q1_13",
		seq_window_agg_ema(volume@Q1_13, 7) as "ema_Q1_13"
FROM Quote WHERE symbol='SYM0';

-- atr
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
		volume@Q1_13 as "Volume_Q1_13",
		seq_window_agg_atr(volume@Q1_13, 7) as "atr_Q1_13"
FROM Quote WHERE symbol='SYM0';

-- Order_by
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
	   seq_sort(volume@Q1_13, 'asc') as by_volume, 
	   seq_order_by(close, by_volume) as "close order_by_vloume"
FROM Quote WHERE symbol='SYM0';

-- project
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
	   seq_project(close, Q1_13) as "close_project"
FROM Quote WHERE symbol='SYM0';

-- Search
SELECT symbol, seq_search(day, 20130101, 20130331) as "Q1_13"
FROM Quote WHERE symbol='SYM0';

-- Subseq
SELECT symbol, seq_subseq(seq_search(day, 20130101, 20131231),1,4) as "2013_1-4"
FROM Quote WHERE symbol='SYM0';

-- _top_max, _top_min, _top_pos_max, _top_pos_min
create table SimpleSequence11( testNumber unsigned(4) primary key, 
dVal1 sequence(double), dVal2 sequence(double) );
INSERT INTO SimpleSequence11(testNumber,dVal1,dVal2)
VALUES(1,[1,2,3,4,5,6,7,8,9], [1,2,3,4,5,6,7,8,9]);

SELECT dVal1, dVal2, seq_top_max(dVal1,3) as "top_max", 
seq_top_pos_max(dVal1,3) as "top_pos_max",
	seq_top_min(dVal2,3) as "top_min", 
seq_top_pos_min(dVal2,3) as "top_pos_min"
FROM SimpleSequence11 WHERE testNumber=1;
