-- hash aggregate
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
	   volume@Q1_13 as "Volume_Q1_13"
FROM Quote WHERE symbol = 'SYM0';

-- _max, _min, _sum, _prd, _avg, _var, _dev
SELECT symbol, seq_search(day, 20130101, 20130331) as Q1_13,
		volume@Q1_13 as "Volume_Q1_13",
		seq_cum_agg_max(volume@Q1_13) as "max_Q1_13", 
		seq_cum_agg_min(volume@Q1_13) as "min_Q1_13",
		seq_cum_agg_sum(volume@Q1_13) as "sum_Q1_13", 
		seq_cum_agg_prd(volume@Q1_13) as "prd_Q1_13", 
		seq_cum_agg_avg(volume@Q1_13) as "avg_Q1_13",
		seq_cum_agg_var(volume@Q1_13) as "var_Q1_13", 
		seq_cum_agg_dev(volume@Q1_13) as "dev_Q1_13"
FROM Quote WHERE symbol='SYM0';
