-- iterate
select symbol, high from Quote;
-- interval
select flattened symbol, seq_search(day, 20130101, 20130401) as quarter, open@quarter, close@quarter from Quote;
-- apply
select symbol, (high+low)/2 from Quote;
-- filter
select symbol, seq_search(day, seq_last_int(day)/100*100+1, seq_last_int(day)) as month, seq_filter(close@month > open@month, month) from Quote;
-- correlation
select symbol, seq_corr(low, high) from Quote;
-- grand aggregate
select symbol, seq_max(close) from Quote;
-- window aggregate
select symbol, seq_map(day, seq_cross(seq_window_agg_avg(close, 20) - seq_window_agg_avg(close, 5), 1)) from Quote;
-- grid aggregate
select symbol symbol, seq_grid_agg_max(close, 7) from Quote;
-- group-by aggregate
select symbol, seq_search(day, 20130101, 20131231) as year, seq_group_agg_sum(volume@year, year/100) from Quote;
-- hash aggregate
select symbol, seq_hash_agg_avg(close, volume/10) as price_per_volume, seq_hash_group_by(price_per_volume) from Quote;
-- sort
select symbol, seq_sort(volume, 'desc') as desc_volumes, close@desc_volumes from Quote;
-- top
select symbol, seq_top_pos_max(close, 10) as top_prices, close@top_prices, volume@top_prices from Quote;
-- VWAP
select symbol, seq_search(day, seq_last_int(day)-7, seq_last_int(day)) as week, seq_sum(close@week*volume@week)/seq_sum(volume@week) from Quote;
