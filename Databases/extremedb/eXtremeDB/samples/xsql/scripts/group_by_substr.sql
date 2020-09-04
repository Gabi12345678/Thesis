create table rates( c varchar using index, rate double );
insert into rates (c,rate) values ( 'u-a', 10.0 );
insert into rates (c,rate) values ( 'u-b', 11.0 );
insert into rates (c,rate) values ( 'u-c', 12.0 );
select * from rates;
select substr(c,3,1) from rates;
select substr(c,3,1), avg(rate) from rates where c like 'u%' group by substr(c,3,1);
select substr(c,3,1), avg(rate) from rates where c like 'u%' group by c;
