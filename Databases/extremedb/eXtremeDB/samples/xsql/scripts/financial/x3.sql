SELECT flattened symbol, seq_search(day, 20130101, 20130131) as Jan_2013, 
       open@Jan_2013 as "open", close@Jan_2013 as "close" 
FROM Quote WHERE symbol='IBM';
