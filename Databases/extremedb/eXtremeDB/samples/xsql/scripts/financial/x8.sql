SELECT symbol, seq_search(day, 20130225, 20130331) as Mar_2013,
		close@Mar_2013 as "Close",
		seq_window_agg_avg(close@Mar_2013, 5) as "5-day-average"
FROM Quote WHERE symbol='IBM';
