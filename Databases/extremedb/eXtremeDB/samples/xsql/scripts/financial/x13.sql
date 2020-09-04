SELECT symbol, seq_top_pos_max(close, 10) as top_pos, 
	close@top_pos as "top-10-close", volume@top_pos as "top-10-volume"
FROM Quote WHERE symbol='IBM';
