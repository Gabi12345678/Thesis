SELECT symbol, seq_search(day, 20130101, 20130331) as trade_date, 
       volume@trade_date as "Volume", 
       seq_group_agg_sum(volume@trade_date, trade_date/100) as "month_vol"
FROM Quote WHERE symbol='IBM';
