create view ATR as select symbol,
   seq_window_agg_atr(seq_maxof(high-low,
                                0||seq_maxof(abs(high[1:] - close),
                                             abs(low[1:] - close))),
                      14)[13:]
from Quote;

create view RSI as select symbol,
      100-(100/(1+(seq_window_agg_ema(seq_maxof(seq_diff(close)[1:], 0), 14)
                   /seq_window_agg_ema(seq_maxof(-seq_diff(close)[1:], 0), 14))))
from Quote;

select * from ATR where symbol = 'SYM1';
select * from RSI where symbol = 'SYM1';
