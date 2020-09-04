-- Display Jan_13 and Volume_Jan_13
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
	   volume@Q1_13 as "Volume_Q1_13"
FROM Quote WHERE symbol = 'SYM0';

-- _max, _min 
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
		volume@Q1_13 as "Volume_Q1_13",
		seq_grid_agg_max(volume@Q1_13, 7) as "max_Q1_13", 
		seq_grid_agg_min(volume@Q1_13, 7) as "min_Q1_13"
FROM Quote WHERE symbol='SYM0';

-- _sum, _avg
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
		volume@Q1_13 as "Volume_Q1_13",
		seq_grid_agg_sum(volume@Q1_13, 7) as "sum_Q1_13", 
		seq_grid_agg_avg(volume@Q1_13, 7) as "avg_Q1_13"
FROM Quote WHERE symbol='SYM0';

-- _var, _dev
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
		volume@Q1_13 as "Volume_Q1_13",
		seq_grid_agg_var(volume@Q1_13, 7) as "var_Q1_13", 
		seq_grid_agg_dev(volume@Q1_13, 7) as "dev_Q1_13"
FROM Quote WHERE symbol='SYM0';
