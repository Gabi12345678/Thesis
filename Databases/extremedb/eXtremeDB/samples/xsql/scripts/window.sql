create table pair (x integer, y integer);
insert into pair values (1,10), (1,20), (2,20), (3, 20), (4,40), (4,50), (5,50), (5,50), (6,60), (6,60), (6, 70), (7, 70), (8, 70), (8, 80), (9, 90);

select x,y,sum(x) over () from pair;
select x,y,sum(y) over (order by x rows between unbounded preceding and unbounded following) from pair;

select x,y,row_number() over w, rank() over w, dense_rank() over w, percent_rank() over w, sum(x) over w from pair window w as (order by x rows between unbounded preceding and unbounded following);
select x,y,row_number() over w, rank() over w, dense_rank() over w, percent_rank() over w, sum(x) over w from pair window w as (partition by x order by y rows between unbounded preceding and unbounded following);
select x,y,row_number() over w, rank() over w, dense_rank() over w, percent_rank() over w, sum(x) over w from pair window w as (partition by y order by x rows between unbounded preceding and unbounded following);

select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (order by x rows between unbounded preceding and current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by x order by y rows between unbounded preceding and current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by y order by x rows between unbounded preceding and current row);

select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (order by x range between unbounded preceding and current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by x order by y range between unbounded preceding and current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by y order by x range between unbounded preceding and current row);

select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (order by x);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by x order by y);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by y order by x);

select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (order by x rows current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by x order by y rows current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by y order by x rows current row);

select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (order by x range current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by x order by y range current row);
select x,y,row_number() over w, rank() over w, dense_rank() over w, sum(x) over w from pair window w as (partition by y order by x range current row);

create table trades(symbol string, day date, price real, volume bigint);
insert into trades values ('AAA', '17.06.2016', 10.1, 101);
insert into trades values ('AAB', '17.06.2016', 20.1, 201);
insert into trades values ('AAC', '17.06.2016', 30.1, 301);
insert into trades values ('AAA', '20.06.2016', 10.2, 102);
insert into trades values ('AAB', '20.06.2016', 20.2, 202);
insert into trades values ('AAC', '20.06.2016', 30.2, 302);
insert into trades values ('AAA', '21.06.2016', 10.3, 103);
insert into trades values ('AAB', '21.06.2016', 20.3, 203);
insert into trades values ('AAC', '21.06.2016', 30.3, 303);
insert into trades values ('AAA', '22.06.2016', 10.4, 104);
insert into trades values ('AAB', '22.06.2016', 20.4, 204);
insert into trades values ('AAC', '22.06.2016', 30.4, 304);
insert into trades values ('AAA', '23.06.2016', 10.5, 105);
insert into trades values ('AAB', '23.06.2016', 20.5, 205);
insert into trades values ('AAC', '23.06.2016', 30.5, 305);
insert into trades values ('AAA', '24.06.2016', 10.6, 106);
insert into trades values ('AAB', '24.06.2016', 20.6, 206);
insert into trades values ('AAC', '24.06.2016', 30.6, 306);
insert into trades values ('AAA', '27.06.2016', 10.7, 107);
insert into trades values ('AAB', '27.06.2016', 20.7, 207);
insert into trades values ('AAC', '27.06.2016', 30.7, 307);

create index pk on trades(symbol,day);

explain select symbol,day,count(*) over w, row_number() over w, rank() over w, dense_rank() over w, avg(price) over w, sum(price) over w, max(price) over w, min(price) over w, median(price) over w, first_value(price) over w, last_value(price) over w, nth_value(price, 2) over w, lag(price) over w, lead(price, 2) over w from trades window w as (partition by symbol order by day);

select symbol,day,count(*) over w, row_number() over w, rank() over w, dense_rank() over w, avg(price) over w, sum(price) over w, max(price) over w, min(price) over w, median(price) over w, first_value(price) over w, last_value(price) over w, nth_value(price, 2) over w, lag(price) over w, lead(price, 2) over w from trades window w as (partition by symbol order by day);

select symbol,day,count(*) over w, row_number() over w, rank() over w, dense_rank() over w, avg(volume) over w, sum(volume) over w, max(volume) over w, min(volume) over w, median(volume) over w, first_value(volume) over w, last_value(volume) over w, nth_value(volume, 2) over w, lag(volume) over w, lead(volume, 2) over w  from trades window w as (partition by symbol order by day);

select symbol,day,avg(price) over (partition by symbol order by day rows between 2 preceding and current row) from trades;
select symbol,day,sum(volume) over (partition by symbol order by day rows between unbounded preceding and unbounded following) from trades;
select symbol,day,sum(volume) over (partition by symbol order by day rows between current row and unbounded following) from trades;
select symbol,day,sum(price) over (partition by symbol order by day range between 2d preceding and 2d following) from trades;



