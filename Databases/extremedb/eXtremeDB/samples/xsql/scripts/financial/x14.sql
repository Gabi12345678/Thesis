SELECT symbol, seq_search(day, seq_last_int(day)-5, seq_last_int(day)) as last_week,
		close@last_week as "Close", volume@last_week as "Volume",
		seq_sum(close@last_week*volume@last_week)/seq_sum(volume@last_week) as "vwap" 
FROM Quote WHERE symbol='IBM';

