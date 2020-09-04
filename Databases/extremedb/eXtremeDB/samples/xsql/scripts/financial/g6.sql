-- Display Jan_13, Volume_Jan_13 and Jan_13_div_7
SELECT symbol, 
	   seq_search(day, 20130101, 20130131) as Jan_13,
	   volume@Jan_13 as "Volume_Jan_13",
	   Jan_13/7 as "Jan_13_div_7"
FROM Quote WHERE symbol = 'SYM0';

-- _max, _min
SELECT symbol, 
	   seq_search(day, 20130101, 20130131) as Jan_13,
	   seq_group_agg_max(volume@Jan_13, Jan_13/7) as "week_max",
	   seq_group_agg_min(volume@Jan_13, Jan_13/7) as "week_min"
FROM Quote WHERE symbol = 'SYM0';

-- _first, _last
SELECT symbol, 
	   seq_search(day, 20130101, 20130131) as Jan_13,
	   seq_group_agg_first(volume@Jan_13, Jan_13/7) as "week_first",
	   seq_group_agg_last(volume@Jan_13, Jan_13/7) as "week_last"
FROM Quote WHERE symbol = 'SYM0';

-- _sum, _avg
SELECT symbol, 
	   seq_search(day, 20130101, 20130131) as Jan_13,
	   seq_group_agg_sum(volume@Jan_13, Jan_13/7) as "week_sum",
	   seq_group_agg_avg(volume@Jan_13, Jan_13/7) as "week_avg"
FROM Quote WHERE symbol = 'SYM0';

-- _var, _dev
SELECT symbol, 
	   seq_search(day, 20130101, 20130131) as Jan_13,
	   seq_group_agg_var(volume@Jan_13, Jan_13/7) as "week_var",
	   seq_group_agg_dev(volume@Jan_13, Jan_13/7) as "week_dev"
FROM Quote WHERE symbol = 'SYM0';

-- _count, _approxdc
SELECT symbol, 
	   seq_search(day, 20130101, 20130131) as Jan_13,
	   seq_group_agg_count(Jan_13/7) as "week_count",
	   seq_group_agg_approxdc(volume@Jan_13, Jan_13/7) as "week_approxdc"
FROM Quote WHERE symbol = 'SYM0';

