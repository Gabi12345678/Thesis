SELECT symbol, high, low, seq_corr(low, high) as "correlation"
FROM Quote WHERE symbol='IBM';
