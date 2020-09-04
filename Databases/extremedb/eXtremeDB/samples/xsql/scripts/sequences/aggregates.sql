create table trade(symbol string, day date, price sequence(real));
insert into trade values ('AAA', '24.05.2017', [10,20,30,40]);
insert into trade values ('AAA', '25.05.2017', [50,60,70,80]);
-- should fail
-- select symbol,seq_sum(seq_concat(price)) from trade group by symbol;

set implicit_aggregates true;
select symbol,seq_sum(seq_concat(price)) from trade group by symbol;
