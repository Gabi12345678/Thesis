echo
create table Pair(x integer, y integer);
insert into Pair values (1,1);
insert into Pair values (2,1);
insert into Pair values (1,2);
insert into Pair values (2,2);
insert into Pair values (2,3);
insert into Pair values (1,3);
select * from Pair order by x distinct, y asc;
select * from Pair order by x distinct, y desc;
explain select * from Pair order by x distinct, y desc;
create index xy on Pair(x,y);
select * from Pair order by x distinct, y asc;
select * from Pair order by x desc distinct, y desc;
explain select * from Pair order by x desc distinct, y desc;

