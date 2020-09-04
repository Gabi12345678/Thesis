-- delete all records previously inserted into table Quote
delete from Quote;

--create temporary table QuoteIn for inporting from csv file
create table QuoteIn(symbol char(21), day char(9), low char(10), high char(10), open char(10), close char(10), volume char(8));

-- import data from IBM_q1_2013.csv
insert or update into Quote(
       symbol, day, low, high, open, close, volume
       )
       select symbol, day, 
            low,
            high,
            open,
            close,
            volume
        from foreign table (path='IBM-q1-2013.csv', skip=1) as QuoteIn;


-- set date, numerical format and show open close values for Q1
numformat "%6.2f"
dtformat "%d/%m/%Y %I:%M:%S %p"

-- show IBM Q1 2013 data
--SELECT flattened symbol, day, low, high, open, close, volume 
--FROM Quote WHERE symbol='IBM';

seqformat short 0 39

-- execute a tight SQL select
SELECT  seq_cross( seq_sub( seq_window_agg_avg(close, 5), seq_window_agg_avg(close, 21) ), 1) as crosspoints,
		close@crosspoints as "Close at 5-21 day MA intersect",
		day@crosspoints as "date"
FROM Quote WHERE symbol='IBM';

-- execute a more explicit SQL select for demonstration purposed
SELECT  day, close,
	    seq_window_agg_avg(close, 5) as ma5,
		seq_window_agg_avg(close, 21) as ma21,
		seq_sub(ma5, ma21) as ma5_21_delta,
		seq_cross(ma5_21_delta, 1) as crosspoints,
		day@crosspoints as "cross_day", 
		close@crosspoints as "cross_close"
FROM Quote WHERE symbol='IBM';
