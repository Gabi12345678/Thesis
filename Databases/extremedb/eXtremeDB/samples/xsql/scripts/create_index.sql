create table t1(s string, i integer);
create table t2(s string, i integer);
create table t3(s string, i integer);
insert into t1 values (1,1);
insert into t2 values (1,1);
insert into t3 values (1,1);
create index t3_pk on t3(s desc, i desc);
create unique index t2_pk on t2(i,s);
create unique index t1_pk on t1(i) using hash;
create index t1_sk on t1(s);
insert into t1 values (2,2);
insert into t2 values (2,2);
insert into t3 values (2,2);
trace on
select * from t1 where i=1;
select * from t1 where s='2';
select * from t3 order by s desc, i desc;
select * from t2 where i=2 and s=2;
select * from t3 where i=1 and s=1;
select * from t2 order by i;
select * from t2 order by i desc;
select * from Statistic;
select * from IndexStatistic;

