SELECT symbol, close, seq_max(close) as "max" 
FROM Quote WHERE symbol='IBM';
