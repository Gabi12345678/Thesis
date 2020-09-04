SELECT symbol, seq_search(day, 20130301, 20130331) as Mar_2013,
		close@Mar_2013 as "Close",
		seq_grid_agg_min(close@Mar_2013, 5) as "Max"
FROM Quote WHERE symbol='IBM';
