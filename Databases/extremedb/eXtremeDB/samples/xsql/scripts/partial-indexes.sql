create table t (i int not null, j int not null);
insert into  t values (1,2),(1,3),(1,4),(1,5);
create index idx on t (i, j);
explain select * from t where i=1 and j > 3 and j < 5;
select * from t where i=1 and j > 2 and j < 5;

create table t2(x integer primary key);
insert into t2 values (0),(1),(2),(3),(4),(5);
explain select * from t2 where x > 1 and x < 5;
select * from t2 where x > 1 and x < 5;

