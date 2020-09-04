SELECT symbol, seq_search(day, 20130201, 20130228) as Feb_2013,
       close@Feb_2013 as Feb_Close, open@Feb_2013 as Feb_Open,
       seq_filter(Feb_Close > Feb_Open, Feb_2013) as "up_Feb",
       seq_filter(Feb_Close > Feb_Open, Feb_Open) as "up_Open",
       seq_filter(Feb_Close > Feb_Open, Feb_Close) as "up_Close"
FROM Quote WHERE symbol='IBM';

