create table KeyValue (k integer using index, v integer);

insert into KeyValue values (1,10);
insert into KeyValue values (3,30);
insert into KeyValue values (2,20);
insert into KeyValue values (5,50);
insert into KeyValue values (4,40);

trace on

select * from KeyValue where k >= 2 order by k asc;
select * from KeyValue where k > 2 order by k asc;
select * from KeyValue where k <= 3 order by k desc;
select * from KeyValue where k < 3 order by k desc;

