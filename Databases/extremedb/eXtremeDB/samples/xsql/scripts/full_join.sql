create table t1(a integer, b integer);
create table t2(c integer, d integer);
insert into t1 values (1,11),(3,31),(3,32),(5,51);
insert into t2 values (1,101),(2,201),(3,301),(4,401),(4,402);
select * from t1 full outer join t2 on a=c;
with s1 as (select a,b from t1),  s2 as (select c,d from t2) select l.*,r.* from s1 as l outer join s2 as r on l.a=r.c;
with s1 as (select a,b from t1),  s2 as (select c,d from t2) select l.*,r.* from s1 as l full outer join s2 as r on l.a=r.c;

