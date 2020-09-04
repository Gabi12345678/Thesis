create table t1 (s string);
create table t2 (s string);
insert into t1 values ('Hello');
insert into t2 values ('world');
select * from ((select * from t1) union all (select * from t2)) tt;