set append_mode true;

create table Quotes(symbol varchar primary key, day sequence(date asc), open sequence(float), close sequence(float));

insert into Quotes values ('AAA', ['05/17/2015','05/18/2015','05/17/2017','05/18/2017','05/19/2017'], [11,12,13,14,15],[21,22,23,24,25]);
insert into Quotes values ('AAB', ['05/17/2015','05/18/2015','05/17/2017','05/18/2017','05/19/2017'], [31,32,33,34,35],[41,42,43,44,45]);
insert into Quotes values ('AAC', ['05/17/2015','05/18/2015','05/17/2017','05/18/2017','05/19/2017'], [51,52,53,54,55],[61,62,63,64,65]);

select * from Quotes;

create table QuoteHor(symbol varchar, day date, open float, close float);
insert into QuoteHor values ('AAA', '05/17/2016', 101, 102), ('AAA', '05/18/2016', 103, 104), ('AAB', '05/17/2016', 105, 106), ('AAB', '05/18/2016', 107, 108);

set append_mode true;

insert or update into Quotes select * from QuoteHor order by symbol,day;

select * from Quotes;

select !seq_search(day, cast('01/01/2015' as date), cast('12/31/2015' as date)) as year, seq_remove(day@year), seq_remove(open@year), seq_remove(close@year) from Quotes for update;

select * from Quotes;

create table Trades(symbol varchar, day integer, exch char(5), ts sequence(integer asc), open sequence(float), close sequence(float));
create unique index TradePK on Trades(symbol,day);

insert into Trades values ('AAA', 20170522, 'NYSE', [100001,100002,101001,101002,101003], [11,12,13,14,15],[21,22,23,24,25]);
insert into Trades values ('AAB', 20170522, 'NYSE', [100001,100002,101001,101002,101003], [11,12,13,14,15],[21,22,23,24,25]);
insert into Trades values ('AAA', 20170523, 'NYSE', [100001,100002,101001,101002,101003], [11,12,13,14,15],[21,22,23,24,25]);

select * from Trades;

create table TradeHor(symbol varchar, day integer, exch char(5), ts integer, open float, close float);
insert into TradeHor values ('AAA', 20170522, 'NYSE', 100501, 101, 102), 
                            ('AAA', 20170522, 'NYSE', 100502, 103, 104), 
                            ('AAB', 20170522, 'NYSE', 100503, 105, 106), 
                            ('AAB', 20170522, 'NYSE', 100504, 107, 108);

insert or update into Trades select * from TradeHor order by symbol,day,ts;

select * from Trades;

select !seq_search(ts, 100000, 100100) as period, seq_remove(ts@period), seq_remove(open@period), seq_remove(close@period) from Trades where symbol='AAA' and day=20170522 for update;

select * from Trades;

insert or update into Trades values ('AAA', 20170522, 'NYSE', [100101, 100102, 100103], [211,212,213],[321,322,323]),
	                               ('AAC', 20170522, 'NYSE', [100101, 100102, 100103], [411,412,413],[521,522,523]),
	                               ('AAB', 20170522, 'NYSE', [100101, 100102, 100103], [611,612,613],[721,722,723]);

select * from Trades;

insert or update into Trades values ('AAA', 20170522, 'NYSE', 100104, 1001, 1002),
	                               ('AAC', 20170522, 'NYSE', 100104, 2001, 2002),
	                               ('AAB', 20170522, 'NYSE', 100104, 3001, 3002);

select * from Trades;
