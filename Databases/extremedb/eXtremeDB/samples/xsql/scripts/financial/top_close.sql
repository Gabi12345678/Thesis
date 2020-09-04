select symbol, seq_top_pos_max(close, 1) as top_index, close@top_index  as "top_Close", volume@top_index as "volume" from Quote;
-- input console