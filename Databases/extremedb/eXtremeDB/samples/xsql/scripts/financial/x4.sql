SELECT flattened symbol, seq_search(day, 20130101, 20130131) as Jan_2013,
		high@Jan_2013 as "High", low@Jan_2013 as "Low", (high@Jan_2013 + low@Jan_2013) / 2 as "Average"
FROM Quote WHERE symbol='IBM';
