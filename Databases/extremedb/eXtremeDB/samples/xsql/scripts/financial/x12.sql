SELECT symbol, seq_sort(volume, 'desc') as desc_volumes, close@desc_volumes as "close"
FROM Quote WHERE symbol='IBM';
