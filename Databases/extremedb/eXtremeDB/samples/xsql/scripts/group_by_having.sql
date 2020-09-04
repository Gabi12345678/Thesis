create table t ( n varchar using index, v integer );
insert into t (n,v) values ('a',1);
insert into t (n,v) values ('a',1);
insert into t (n,v) values ('a',1);
insert into t (n,v) values ('b',1);
insert into t (n,v) values ('b',1);
insert into t (n,v) values ('b',1);
insert into t (n,v) values ('c',1);
insert into t (n,v) values ('c',1);
insert into t (n,v) values ('c',1);
select n, sum(v) as sv from t group by n;
select n, sum(v) as sv from t group by n having sv = 3;
select n, sum(v) as sv from t group by n having sum(v) = 3;

