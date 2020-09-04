select symbol, seq_search(day, 20130401, 20130801) as quarter, seq_filter_search(volume@quarter > 10, quarter) : hv, open@hv from Quote;
select symbol, seq_search(day, 20130401, 20130801) as quarter, seq_filter_pos(volume@quarter > 10) : hv, open@quarter@hv from Quote;
select symbol, seq_search(day, 20130401, 20130801) as quarter, seq_filter(volume@quarter > 10, volume@quarter) , seq_filter(volume@quarter > 10, open@quarter) from Quote;
