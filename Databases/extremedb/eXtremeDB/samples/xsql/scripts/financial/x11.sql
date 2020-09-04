SELECT symbol, seq_search(day, 20130304, 20130315) as Mar_2013,
		volume@Mar_2013 as "Volume", close@Mar_2013 as "Close",
	   seq_hash_agg_avg(close@Mar_2013, volume@Mar_2013/1000000) as avg_vol, 
	   seq_hash_group_by(avg_vol) as "group_by"
FROM Quote WHERE symbol='IBM';
